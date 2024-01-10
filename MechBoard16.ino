/*******************************************************************************
 * This file is part of MechBoard16.                                           *
 *                                                                             *
 * Copyright (C) 2024 by SukkoPera <software@sukkology.net>                    *
 *                                                                             *
 * MechBoard16 is free software: you can redistribute it and/or                *
 * modify it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or           *
 * (at your option) any later version.                                         *
 *                                                                             *
 * MechBoard16 is distributed in the hope that it will be useful,              *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
 * GNU General Public License for more details.                                *
 *                                                                             *
 * You should have received a copy of the GNU General Public License           *
 * along with MechBoard16. If not, see http://www.gnu.org/licenses.            *
 ******************************************************************************/

// Send debug messages to serial port
#define ENABLE_SERIAL_DEBUG

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

#include "CircularBuffer.h"
struct KeyMatrixSample {
	byte rows;			// If this is made a uint8_t we save 6 bytes, how come?!?
	byte cols;
};

/* volatile */ CircularBuffer<KeyMatrixSample, byte, 32> matrixSamples;

#include "LedControl.h"
LedControl lc (PIN_MAX7221_DATA, PIN_MAX7221_CLK, PIN_MAX7221_SEL, 1 /* Number of MAX72xx chips */);

unsigned long DELAY_TIME = 35;

enum class Key: byte {
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
	RETURN,
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
	SPACE,

	// Fake key placeholder
	NONE = 0xFF
};

const byte N_PHYSICAL_KEYS = static_cast<byte> (Key::SPACE) + 1;

enum class Mode: byte {
	ALWAYS_OFF,
	ALWAYS_ON,
	PRESSED_ON,
	PRESSED_OFF
};

Mode mode = Mode::PRESSED_OFF;

byte animation = 0;

/*******************************************************************************
 * END OF SETTINGS
 ******************************************************************************/

#include <EEPROM.h>
constexpr word EEP_ANIMATION = 0x100;
constexpr word EEP_MODE = 0x101;

#include <avr/pgmspace.h>

#ifdef ENABLE_SERIAL_DEBUG
	#define dstart(spd) Serial.begin (spd)
	#define debug(...) Serial.print (__VA_ARGS__)
	#define debugln(...) Serial.println (__VA_ARGS__)
#else
	#define dstart(...)
	#define debug(...)
	#define debugln(...)
#endif

struct MatrixCoordinates {
	byte row;
	byte col;
};

/* Maps a key to the (row, col) tuple that controls its led.
 * Built by buildLedCoordinates().
 */
MatrixCoordinates ledCoordinates[N_PHYSICAL_KEYS];

/* Maps a key to the (row, col) tuple describing its position in the keyboard matrix, useful for quick lookups.
 * Built by buildKeyCoordinates().
 */
MatrixCoordinates keyCoordinates[N_PHYSICAL_KEYS];

// Current state of the keyboard matrix, true if a key is pressed
boolean keyboardMatrix[MATRIX_ROWS][MATRIX_COLS] = {false};

//! Number of physical rows of keys
constexpr byte N_PHYSICAL_ROWS = 5;

constexpr byte splash0nCols = 18;

// Lists of keys per each row, somehow lined up in "diagonal columns"
const Key splash0row4[splash0nCols] PROGMEM = {Key::NONE,    Key::ESC,   Key::_1,   Key::_2,    Key::_3,   Key::_4,   Key::_5,    Key::_6,   Key::_7,   Key::_8,    Key::_9,     Key::_0,    Key::LEFT,      Key::RIGHT,    Key::UP,     Key::DOWN,   Key::DEL,   Key::F1};
const Key splash0row3[splash0nCols] PROGMEM = {Key::NONE,    Key::CTRL,  Key::Q,    Key::W,     Key::E,    Key::R,    Key::T,     Key::Y,    Key::U,    Key::I,     Key::O,      Key::P,     Key::AT,        Key::PLUS,     Key::MINUS,  Key::NONE,   Key::CLEAR, Key::F2};
const Key splash0row2[splash0nCols] PROGMEM = {Key::RUNSTOP, Key::NONE,  Key::A,    Key::S,     Key::D,    Key::F,    Key::G,     Key::H,    Key::J,    Key::K,     Key::L,      Key::COLON, Key::SEMICOLON, Key::ASTERISK, Key::NONE,   Key::RETURN, Key::NONE,  Key::F3};
const Key splash0row1[splash0nCols] PROGMEM = {Key::CMD,     Key::SHIFT, Key::Z,    Key::X,     Key::C,    Key::V,    Key::B,     Key::N,    Key::M,    Key::COMMA, Key::PERIOD, Key::SLASH, Key::NONE,      Key::SHIFT,    Key::POUND,  Key::EQUAL,  Key::NONE,  Key::HELP};
const Key splash0row0[splash0nCols] PROGMEM = {Key::NONE,    Key::NONE,  Key::NONE, Key::SPACE, Key::NONE, Key::NONE, Key::SPACE, Key::NONE, Key::NONE, Key::SPACE, Key::NONE,   Key::NONE,  Key::NONE,      Key::NONE,     Key::NONE,   Key::NONE,   Key::NONE,  Key::NONE};

constexpr Key const * splash0rows[N_PHYSICAL_ROWS] PROGMEM = {splash0row4, splash0row3, splash0row2, splash0row1, splash0row0};

void splash0 () {
	for (byte i = 0; i < splash0nCols; ++i) {
		// Turn on a column...
		for (byte j = 0; j < N_PHYSICAL_ROWS; ++j) {
			const byte* krow = pgm_read_byte (&splash0rows[j]);
			const byte k = pgm_read_byte (&(krow[i]));
			if (static_cast<Key> (k) != Key::NONE) {
				const MatrixCoordinates& pos = ledCoordinates[k];
				lc.setLed (0, pos.row, pos.col, true);
			}
		}

		// ... wait a bit...
		delay (60);

		// ... and turn it off
		for (byte j = 0; j < N_PHYSICAL_ROWS; ++j) {
			const byte* krow = pgm_read_byte (&splash0rows[j]);
			const byte k = pgm_read_byte (&(krow[i]));
			if (static_cast<Key> (k) != Key::NONE) {
				const MatrixCoordinates& pos = ledCoordinates[k];
				lc.setLed (0, pos.row, pos.col, false);
			}
		}
	}
}

// TODO: Move all the following tables to flash memory

// Order in which they appear on the keyboard (+1 for 2 Shifts)
constexpr Key splash_order[N_PHYSICAL_KEYS + 1] = {
	Key::ESC, Key::_1, Key::_2, Key::_3, Key::_4, Key::_5, Key::_6, Key::_7, Key::_8, Key::_9, Key::_0, Key::LEFT, Key::RIGHT, Key::UP, Key::DOWN, Key::DEL,
	Key::CTRL, Key::Q, Key::W, Key::E, Key::R, Key::T, Key::Y, Key::U, Key::I, Key::O, Key::P, Key::AT, Key::PLUS, Key::MINUS, Key::CLEAR,
	Key::RUNSTOP, /* Shift Lock */ Key::A, Key::S, Key::D, Key::F, Key::G, Key::H, Key::J, Key::K, Key::L, Key::COLON, Key::SEMICOLON, Key::ASTERISK, Key::RETURN,
	Key::CMD, Key::SHIFT, Key::Z, Key::X, Key::C, Key::V, Key::B, Key::N, Key::M, Key::COMMA, Key::PERIOD, Key::SLASH, Key::SHIFT, Key::POUND, Key::EQUAL,
	Key::SPACE,
	Key::HELP, Key::F3, Key::F2, Key::F1    // Reverse order just to be cool ;)
};
	
void splash1 () {
	for (byte i = 0; i < N_PHYSICAL_KEYS + 1; ++i) {
		const MatrixCoordinates& pos = ledCoordinates[static_cast<int> (splash_order[i])];
		lc.setLed (0, pos.row, pos.col, true);
		delay (DELAY_TIME);
		lc.setLed (0, pos.row, pos.col, false);
	}
}

constexpr Key keymap[MATRIX_ROWS][MATRIX_COLS] = {
	{Key::DEL,  Key::RETURN,   Key::POUND,     Key::HELP,  Key::F1,     Key::F2,    Key::F3,    Key::AT},
	{Key::_3,   Key::W,        Key::A,         Key::_4,    Key::Z,      Key::S,     Key::E,     Key::SHIFT},
	{Key::_5,   Key::R,        Key::D,         Key::_6,    Key::C,      Key::F,     Key::T,     Key::X},
	{Key::_7,   Key::Y,        Key::G,         Key::_8,    Key::B,      Key::H,     Key::U,     Key::V},
	{Key::_9,   Key::I,        Key::J,         Key::_0,    Key::M,      Key::K,     Key::O,     Key::N},
	{Key::DOWN, Key::P,        Key::L,         Key::UP,    Key::PERIOD, Key::COLON, Key::MINUS, Key::COMMA},
	{Key::LEFT, Key::ASTERISK, Key::SEMICOLON, Key::RIGHT, Key::ESC,    Key::EQUAL, Key::PLUS,  Key::SLASH},
	{Key::_1,   Key::CLEAR,    Key::CTRL,      Key::_2,    Key::SPACE,  Key::CMD,   Key::Q,     Key::RUNSTOP}
};

// Only useful for debugging
constexpr char KEY_NAMES[MATRIX_ROWS][MATRIX_COLS][4] = {
	{"DEL", "RET", "£",   "HLP", "F1",  "F2", "F3", "@"},
	{"3",   "W",   "A",   "4",   "Z",   "S",  "E",  "SHF"},
	{"5",   "R",   "D",   "6",   "C",   "F",  "T",  "X"},
	{"7",   "Y",   "G",   "8",   "B",   "H",  "U",  "V"},
	{"9",   "I",   "J",   "0",   "M",   "K",  "O",  "N"},
	{"DN",  "P",   "L",   "UP",  ".",   ":",  "-",  ","},
	{"LF",  "*",   ";",   "RT",  "ESC", "=",  "+",  "/"},
	{"1",   "CLR", "CTL", "2",   "SPC", "C=", "Q",  "RUN"}
};

// Populates the ledCoordinates array
boolean buildLedCoordinates () {
	bool found;
	
	for (byte i = 0; i < N_PHYSICAL_KEYS; ++i) {
		Key k = static_cast<Key> (i);

		found = false;
		for (byte row = 0; row < MATRIX_ROWS && !found; ++row) {
			for (byte col = 0; col < MATRIX_COLS && !found; ++col) {
				if (keymap[row][col] == k) {
					/* The led matrix was supposed to be the same as the keyboard matrix. I don't know whether I made a
					 * wiring mistake or if LedControl numbers things differently, but it turns out we need to swap the
					 * coordinates and modify them slightly in order to use them with lc.setLed().
					 */
					ledCoordinates[i].row = col;
					ledCoordinates[i].col = (row + 1) % 8;
					found = true;
				}
			}
		}

		if (!found) {
			break;
		}
	}

	return found;
}

// Populates the keyCoordinates array
boolean buildKeyCoordinates () {
	bool found;
	
	for (byte i = 0; i < N_PHYSICAL_KEYS; ++i) {
		Key k = static_cast<Key> (i);

		found = false;
		for (byte row = 0; row < MATRIX_ROWS && !found; ++row) {
			for (byte col = 0; col < MATRIX_COLS && !found; ++col) {
				if (keymap[row][col] == k) {
					keyCoordinates[i].row = row;
					keyCoordinates[i].col = col;
					found = true;
				}
			}
		}

		if (!found) {
			break;
		}
	}

	return found;
}

// Pin-change ISR, called whenever the C16 polls the keyboard, all we do is snoop what is going on ;)
ISR (PCINT0_vect) {
	KeyMatrixSample ms {
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

// Called when a keypress is detected
void onKeyPressed (const byte row, const byte col) {
	switch (mode) {
		case Mode::PRESSED_ON: {
			const Key k = keymap[row][col];
			const MatrixCoordinates& pos = ledCoordinates[static_cast<int> (k)];
			lc.setLed (0, pos.row, pos.col, true);
			break;
		}
		case Mode::PRESSED_OFF: {
			const Key k = keymap[row][col];
			const MatrixCoordinates& pos = ledCoordinates[static_cast<int> (k)];
			lc.setLed (0, pos.row, pos.col, false);
			break;
		}
		case Mode::ALWAYS_ON:
		case Mode::ALWAYS_OFF:
			// Nothing to do
			break;
	}
}

// Called when a keyrelease is detected
void onKeyReleased (const byte row, const byte col) {
	switch (mode) {
		case Mode::PRESSED_ON: {
			const Key k = keymap[row][col];
			const MatrixCoordinates& pos = ledCoordinates[static_cast<int> (k)];
			lc.setLed (0, pos.row, pos.col, false);
			break;
		}
		case Mode::PRESSED_OFF: {
			const Key k = keymap[row][col];
			const MatrixCoordinates& pos = ledCoordinates[static_cast<int> (k)];
			lc.setLed (0, pos.row, pos.col, true);
			break;
		}
		case Mode::ALWAYS_ON:
		case Mode::ALWAYS_OFF:
			// Nothing to do
			break;
	}
}

void updateLighting () {
	// Update the LED pattern according to the chosen mode
	switch (mode) {
		case Mode::ALWAYS_ON:
			// Turn all leds on
			for (byte i = 0; i < MATRIX_COLS; ++i) {
				lc.setRow (0, i, 0xFF);
			}
			break;
		case Mode::PRESSED_OFF:
			for (byte r = 0; r < MATRIX_ROWS; ++r) {
				for (byte c = 0; c < MATRIX_COLS; ++c) {
					const Key k = keymap[r][c];
					const MatrixCoordinates& pos = ledCoordinates[static_cast<int> (k)];
					lc.setLed (0, pos.row, pos.col, !keyboardMatrix[r][c]);
				}
			}
			break;
		case Mode::ALWAYS_OFF:
			// Turn all leds off
			for (byte i = 0; i < MATRIX_COLS; ++i) {
				lc.setRow (0, i, 0x00);
			}
			break;
		case Mode::PRESSED_ON:
			for (byte r = 0; r < MATRIX_ROWS; ++r) {
				for (byte c = 0; c < MATRIX_COLS; ++c) {
					const Key k = keymap[r][c];
					const MatrixCoordinates& pos = ledCoordinates[static_cast<int> (k)];
					lc.setLed (0, pos.row, pos.col, keyboardMatrix[r][c]);
				}
			}
			break;
	}
}

void onSetMode (const Mode newMode) {
	if (newMode != mode) {
		debug (F("Setting mode "));
		debugln (static_cast<byte> (newMode));
		
		mode = newMode;
		EEPROM.write (EEP_MODE, static_cast<byte> (mode));
		updateLighting ();
	}
}

void onSetAnimation (const int newAnimation) {
	if (newAnimation != animation) {
		debug (F("Setting animation "));
		debugln (newAnimation);
		
		animation = newAnimation;
		EEPROM.write (EEP_ANIMATION, static_cast<byte> (animation));
	}
}

void doAnimation () {
	debug (F("Playing intro animation "));
	debugln (animation);
		
	switch (animation) {
		case 0:
		default:
			splash0 ();
			break;
		case 1:
			splash1 ();
			break;
	}
}

void setup () {
	dstart (115200);
	
	// Wake up and configure the MAX72XX ASAP, since it might show a random pattern at startup
	lc.shutdown (0, false);
	lc.setIntensity (0, 15);		// 0-15
	lc.clearDisplay (0);

	/* Keyboard polling: TED drives the rows, which we have on PORT B, while the keyboard "outputs" the columns, which
	 * we have on PORT D. They are all INPUTs by default se all we have to do is to enable the pin-change interrupts on
	 * all pins of PORT B.
	 */
	PCMSK0 = (1 << PCINT7) | (1 << PCINT6) | (1 << PCINT5) | (1 << PCINT4) |
	         (1 << PCINT3) | (1 << PCINT2) | (1 << PCINT1) | (1 << PCINT0);
	PCICR |= (1 << PCIE0);

	// R/G/B LED pins: configure as OUTPUTs
	pinMode (PIN_LED_R, OUTPUT);
	pinMode (PIN_LED_G, OUTPUT);
	pinMode (PIN_LED_B, OUTPUT);

	// Build the required coordinates array
	if (!buildLedCoordinates () || !buildKeyCoordinates ()) {
		debugln (F("Unable to build the LED coordinates array, this indicates a mistake in the code"));

		// Hang with fast blinking
		while (true) {
			digitalWrite (PIN_LED_R, HIGH);
			delay (222);
			digitalWrite (PIN_LED_R, LOW);
			delay (222);
		}
	}

	// Do the power-up animation
	byte b = EEPROM.read (EEP_ANIMATION);
	if (b < 2) {
		animation = b;
	} else {
		// Default animation
		animation = 0;
	}
	doAnimation ();

	// Prepare the initial LED pattern according to the saved mode
	b = EEPROM.read (EEP_MODE);
	if (b <= static_cast<byte> (Mode::PRESSED_OFF)) {
		mode = static_cast<Mode> (b);
	} else {
		// Default mode
		mode = Mode::PRESSED_OFF;
	}
	updateLighting ();
}

boolean isPressed (const Key k) {
	const MatrixCoordinates pos = keyCoordinates[static_cast<byte> (k)];	// FIXME: Check bounds
	return keyboardMatrix[pos.row][pos.col];
}

void loop () {
	while (matrixSamples.available ()) {
		KeyMatrixSample sample = matrixSamples.get ();

		/* When scanning the keyboard, the C16/+4 KERNAL first does a quick test to check if any key is pressed at all:
		 * it brings all the rows down and checks whether all cols are up or not. If at least one column is down, it
		 * goes on to check every individual row, as at least one key must be pressed and there's no other way to find
		 * out exactly which one, otherwise it takes no further action and just terminates the scan there.
		 */
		if (sample.rows == 0x00 && sample.cols == 0xFF) {
			// All keys released
			for (byte r = 0; r < MATRIX_ROWS; ++r) {
				for (byte c = 0; c < MATRIX_COLS; ++c) {
					if (keyboardMatrix[r][c]) {
						debug (F("Released "));
						debug (r);
						debug (F(","));
						debug (c);
						debug (F(": "));
						debugln (KEY_NAMES[r][c]);

						onKeyReleased (r, c);

						keyboardMatrix[r][c] = false;
					}
				}
			}
		} else if (countSetBits (sample.rows) == 7) {
			// Exactly one row is cleared, find out which one and update all its columns
			for (byte r = 0; r < MATRIX_ROWS; ++r) {
				if ((sample.rows & (1 << r)) == 0) {
					for (byte c = 0; c < MATRIX_COLS; ++c) {
						boolean pressed = (sample.cols & (1 << c)) == 0;
						if (pressed && !keyboardMatrix[r][c]) {
							debug (F("Pressed "));
							debug (r);
							debug (',');
							debug (c);
							debug (F(": "));
							debugln (KEY_NAMES[r][c]);

							onKeyPressed (r, c);
						} else if (!pressed && keyboardMatrix[r][c]) {
							debug (F("Released "));
							debug (r);
							debug (F(","));
							debug (c);
							debug (F(": "));
							debugln (KEY_NAMES[r][c]);

							onKeyReleased (r, c);
						}
						keyboardMatrix[r][c] = pressed;
					}

					break;		// There is necessarily only one row at 0
				}
			}
		}
	}

	// Check combos
	if (isPressed (Key::CMD) && isPressed (Key::CTRL)) {
		if (isPressed (Key::F1)) {
			onSetMode (Mode::ALWAYS_OFF);
		} else if (isPressed (Key::F2)) {
			onSetMode (Mode::ALWAYS_ON);
		} else if (isPressed (Key::F3)) {
			onSetMode (Mode::PRESSED_ON);
		} else if (isPressed (Key::HELP)) {
			onSetMode (Mode::PRESSED_OFF);
		} else if (isPressed (Key::_1)) {
			onSetAnimation (0);
		} else if (isPressed (Key::_2)) {
			onSetAnimation (1);
		} else if (isPressed (Key::RUNSTOP)) {
			// TODO: RESET
		}
	}
	
	//~ digitalWrite (PIN_LED_R, HIGH);
	//~ delay (1000);
	//~ digitalWrite (PIN_LED_R, LOW);
	//~ delay (1000);
}
