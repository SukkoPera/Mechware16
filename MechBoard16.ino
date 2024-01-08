const byte PIN_LED_R = A5;
const byte PIN_LED_G = A4;
const byte PIN_LED_B = A3;
const byte PIN_MAX7221_SEL = A2;
const byte PIN_MAX7221_DATA = A1;
const byte PIN_MAX7221_CLK = A0;

/** \brief Number of rows in the C16/Plus4 keyboard matrix
 */
const byte MATRIX_ROWS = 8;

/** \brief Number of columns in the C16/Plus4 keyboard matrix
 */
const byte MATRIX_COLS = 8;

//~ const byte PIN_ROWS[MATRIX_ROWS] = {17 /* RXLED */, 15 /* SCK */, 16 /* MOSI */, 14 /* MISO */, 8, 9, 10, 11};
//~ const byte PIN_COLS[MATRIX_COLS] = {3, 2, 0, 1, 4, 30 /* TXLED */, 12, 6};

#include "CircularBuffer.h"
struct MatrixSample {
	uint8_t rows;
	uint8_t cols;
};

/* volatile */ CircularBuffer<MatrixSample, uint8_t, 32> matrixSamples;

#include "LedControl.h"
LedControl lc(PIN_MAX7221_DATA, PIN_MAX7221_CLK, PIN_MAX7221_SEL, 1 /* Number of MAX72xx chips */);

unsigned long DELAY_TIME = 35;

enum class Key: uint8_t {
	// Numbers
	_0,
	_1,
	_2,
	_3,
	_4,
	_5,
	_6,
	_7,
	_8,
	_9,
	
	// Letters
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	
	// Function
	F1,
	F2,
	F3,
	HELP,
	
	// Cursor
	UP,
	DOWN,
	LEFT,
	RIGHT,
	
	// Others
	ASTERISK,
	AT,
	CLEAR,
	CMD,
	COLON,
	COMMA,
	CTRL,
	DEL,
	ENTER,
	EQUAL,
	ESC,
	MINUS,
	PERIOD,
	PLUS,
	POUND,
	RUNSTOP,
	SEMICOLON,
	SHIFT,
	SLASH,
	SPACE
};

const byte N_KEYS = static_cast<byte> (Key::SPACE) + 1;

enum class Mode: uint8_t {
	ALWAYS_OFF,
	ALWAYS_ON,
	PRESSED_ON,
	PRESSED_OFF
};

Mode mode = Mode::PRESSED_OFF;


// Order in which they appear on the keyboard (+1 for 2 Shifts)
constexpr Key splash_order[N_KEYS + 1] = {
	Key::ESC, Key::_1, Key::_2, Key::_3, Key::_4, Key::_5, Key::_6, Key::_7, Key::_8, Key::_9, Key::_0, Key::LEFT, Key::RIGHT, Key::UP, Key::DOWN, Key::DEL,
	Key::CTRL, Key::Q, Key::W, Key::E, Key::R, Key::T, Key::Y, Key::U, Key::I, Key::O, Key::P, Key::AT, Key::PLUS, Key::MINUS, Key::CLEAR,
	Key::RUNSTOP, /* Shift Lock */ Key::A, Key::S, Key::D, Key::F, Key::G, Key::H, Key::J, Key::K, Key::L, Key::COLON, Key::SEMICOLON, Key::ASTERISK, Key::ENTER,
	Key::CMD, Key::SHIFT, Key::Z, Key::X, Key::C, Key::V, Key::B, Key::N, Key::M, Key::COMMA, Key::PERIOD, Key::SLASH, Key::SHIFT, Key::POUND, Key::EQUAL,
	Key::SPACE,
	Key::HELP, Key::F3, Key::F2, Key::F1    // Reverse order just to be cool ;)
};

constexpr Key keymap[MATRIX_ROWS][MATRIX_COLS] = {
	//~ {Key::DEL,   Key::_3,    Key::_5, Key::_7, Key::_9, Key::DOWN,   Key::LEFT,      Key::_1},
	//~ {Key::ENTER, Key::W,     Key::R,  Key::Y,  Key::I,  Key::P,      Key::ASTERISK,  Key::CLEAR},
	//~ {Key::POUND, Key::A,     Key::D,  Key::G,  Key::J,  Key::L,      Key::SEMICOLON, Key::CTRL},
	//~ {Key::HELP,  Key::_4,    Key::_6, Key::_8, Key::_0, Key::UP,     Key::RIGHT,     Key::_2},
	//~ {Key::F1,    Key::Z,     Key::C,  Key::B,  Key::M,  Key::PERIOD, Key::ESC,       Key::SPACE},
	//~ {Key::F2,    Key::S,     Key::F,  Key::H,  Key::K,  Key::COLON,  Key::EQUAL,     Key::CMD},
	//~ {Key::F3,    Key::E,     Key::T,  Key::U,  Key::O,  Key::MINUS,  Key::PLUS,      Key::Q},
	//~ {Key::AT,    Key::SHIFT, Key::X,  Key::V,  Key::N,  Key::COMMA,  Key::SLASH,     Key::RUNSTOP}

	{Key::DEL,  Key::ENTER,    Key::POUND,     Key::HELP,  Key::F1,     Key::F2,    Key::F3,    Key::AT},
	{Key::_3,   Key::W,        Key::A,         Key::_4,    Key::Z,      Key::S,     Key::E,     Key::SHIFT},
	{Key::_5,   Key::R,        Key::D,         Key::_6,    Key::C,      Key::F,     Key::T,     Key::X},
	{Key::_7,   Key::Y,        Key::G,         Key::_8,    Key::B,      Key::H,     Key::U,     Key::V},
	{Key::_9,   Key::I,        Key::J,         Key::_0,    Key::M,      Key::K,     Key::O,     Key::N},
	{Key::DOWN, Key::P,        Key::L,         Key::UP,    Key::PERIOD, Key::COLON, Key::MINUS, Key::COMMA},
	{Key::LEFT, Key::ASTERISK, Key::SEMICOLON, Key::RIGHT, Key::ESC,    Key::EQUAL, Key::PLUS,  Key::SLASH},
	{Key::_1,   Key::CLEAR,    Key::CTRL,      Key::_2,    Key::SPACE,  Key::CMD,   Key::Q,     Key::RUNSTOP}
};

#define LED_ROW(mtxR, mtxC) (mtxC)
#define LED_COL(mtxR, mtxC) ((mtxR + 1) % 8)

struct Position {
	byte row;
	byte col;
};

Position positions[N_KEYS];

ISR (PCINT0_vect) {
	MatrixSample ms {
		.rows = PINB,
		.cols = PIND
	};

	if (ms.rows != 0xFF) {
		matrixSamples.put (ms);
	}
}

/** \brief Get number of set bits in the binary representation of a number
 * 
 * All hail to Brian Kernighan.
 * 
 * \param[in] n The number
 * \return The number of bits set
 */
unsigned int countSetBits (int n) { 
	unsigned int count = 0; 

	while (n) { 
		n &= n - 1;
		++count; 
	} 

	return count; 
}

void onKeyPressed (const byte row, const byte col) {
	switch (mode) {
		case Mode::PRESSED_ON:
			lc.setLed(0, LED_ROW (row, col), LED_COL (row, col), true);
			break;
		case Mode::PRESSED_OFF:
			lc.setLed(0, LED_ROW (row, col), LED_COL (row, col), false);
			break;
		case Mode::ALWAYS_ON:
		case Mode::ALWAYS_OFF:
			// Nothing to do
			break;
	}
}

void onKeyReleased (const byte row, const byte col) {
	switch (mode) {
		case Mode::PRESSED_ON:
			lc.setLed(0, LED_ROW (row, col), LED_COL (row, col), false);
			break;
		case Mode::PRESSED_OFF:
			lc.setLed(0, LED_ROW (row, col), LED_COL (row, col), true);
			break;
		case Mode::ALWAYS_ON:
		case Mode::ALWAYS_OFF:
			// Nothing to do
			break;
	}
}

void setup() {
	/*
	 The MAX72XX is in power-saving mode on startup,
	 we have to do a wakeup call
	 */
	lc.shutdown (0, false);
	/* Set the brightness to a medium values */
	lc.setIntensity (0, 15);
	/* and clear the display */
	lc.clearDisplay (0);

	for (byte i = 0; i < N_KEYS; ++i) {
		Key k = static_cast<Key> (i);

		bool found = false;
		byte row, col;
		for (row = 0; row < MATRIX_ROWS && !found; ++row) {
			for (col = 0; col < MATRIX_COLS && !found; ++col) {
				if (keymap[row][col] == k) {
					// Not sure why, but we need to swap the coordinates and modify them slightly
					positions[i].row = LED_ROW (row, col);
					positions[i].col = LED_COL (row, col);
					found = true;
				}
			}
		}

		if (!found) {
			while (true) {
			}
		}
	}

	// Keyboard polling pins

	// TED drives the rows, which we have on PORT B
	//~ for (byte i = 0; i < MATRIX_ROWS; ++i) {
		//~ pinMode (PIN_ROWS[i], INPUT);
	//~ }

	// Enable pin-change interrupts on all pins of PORT B
	PCMSK0 = (1 << PCINT7) | (1 << PCINT6) | (1 << PCINT5) | (1 << PCINT4) |
	         (1 << PCINT3) | (1 << PCINT2) | (1 << PCINT1) | (1 << PCINT0);
	PCICR |= (1 << PCIE0);

	// The keyboard "outputs" the columns, which we have on PORT D
	//~ for (byte i = 0; i < MATRIX_COLS; ++i) {
		//~ pinMode (PIN_COLS[i], INPUT);
	//~ }


	// R/G/B LED pins
	pinMode (PIN_LED_R, OUTPUT);
	pinMode (PIN_LED_G, OUTPUT);
	pinMode (PIN_LED_B, OUTPUT);


	for (byte i = 0; i < N_KEYS + 1; ++i) {
		const Position& pos = positions[static_cast<int> (splash_order[i])];
		lc.setLed(0, pos.row, pos.col, true);
		delay(DELAY_TIME);
		lc.setLed(0, pos.row, pos.col, false);
	}

	switch (mode) {
		case Mode::ALWAYS_ON:
		case Mode::PRESSED_OFF:
			// Turn all leds on
			for (byte i = 0; i < MATRIX_COLS; ++i) {
				lc.setRow (0, i, 0xFF);
			}
			break;
		case Mode::ALWAYS_OFF:
		case Mode::PRESSED_ON:
			// Nothing to do, leds are already all off at this point
			break;
	}

	Serial.begin(115200);
}

boolean matrix[MATRIX_ROWS][MATRIX_COLS] = {false};

const char KEY_NAMES[MATRIX_ROWS][MATRIX_COLS][4] = {
	{"DEL", "RET", "£",   "HLP", "F1",  "F2", "F3", "@"},
	{"3",   "W",   "A",   "4",   "Z",   "S",  "E",  "SHF"},
	{"5",   "R",   "D",   "6",   "C",   "F",  "T",  "X"},
	{"7",   "Y",   "G",   "8",   "B",   "H",  "U",  "V"},
	{"9",   "I",   "J",   "0",   "M",   "K",  "O",  "N"},
	{"DN",  "P",   "L",   "UP",  ".",   ":",  "-",  ","},
	{"LF",  "*",   ";",   "RT",  "ESC", "=",  "+",  "/"},
	{"1",   "CLR", "CTL", "2",   "SPC", "C=", "Q",  "RUN"}
};

void loop () {
	while (matrixSamples.available ()) {
		MatrixSample sample = matrixSamples.get ();

		if (sample.rows == 0x00 && sample.cols == 0xFF) {
			// All keys released
			for (byte r = 0; r < MATRIX_ROWS; ++r) {
				for (byte c = 0; c < MATRIX_COLS; ++c) {
					if (matrix[r][c]) {
						Serial.print ("Released ");
						Serial.print (r);
						Serial.print (',');
						Serial.print (c);
						Serial.print (": ");
						Serial.println (KEY_NAMES[r][c]);

						onKeyReleased (r, c);

						matrix[r][c] = false;
					}
				}
			}
		} else if (countSetBits (sample.rows) == 7) {
			// Exactly one row is cleared, find out which one and update all its columns
			for (byte r = 0; r < MATRIX_ROWS; ++r) {
				if ((sample.rows & (1 << r)) == 0) {
					for (byte c = 0; c < MATRIX_COLS; ++c) {
						boolean pressed = (sample.cols & (1 << c)) == 0;
						if (pressed && !matrix[r][c]) {
							Serial.print ("Pressed ");
							Serial.print (r);
							Serial.print (',');
							Serial.print (c);
							Serial.print (": ");
							Serial.println (KEY_NAMES[r][c]);

							onKeyPressed (r, c);
						} else if (!pressed && matrix[r][c]) {
							Serial.print ("Released ");
							Serial.print (r);
							Serial.print (',');
							Serial.print (c);
							Serial.print (": ");
							Serial.println (KEY_NAMES[r][c]);

							onKeyReleased (r, c);
						}
						matrix[r][c] = pressed;
					}

					break;		// There is necessarily only one row at 0
				}
			}
		}
	}
	
	//~ digitalWrite (PIN_LED_R, HIGH);
	//~ delay (1000);
	//~ digitalWrite (PIN_LED_R, LOW);
	//~ delay (1000);
}
