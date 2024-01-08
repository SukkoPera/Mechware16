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
//~ #define ENABLE_SERIAL_DEBUG

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
	uint8_t rows;
	uint8_t cols;
};

/* volatile */ CircularBuffer<KeyMatrixSample, uint8_t, 32> matrixSamples;

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

/*******************************************************************************
 * END OF SETTINGS
 ******************************************************************************/

#ifdef ENABLE_SERIAL_DEBUG
	//~ #include <avr/pgmspace.h>
	//~ typedef const __FlashStringHelper * FlashStr;
	//~ typedef const byte* PGM_BYTES_P;
	//~ #define PSTR_TO_F(s) reinterpret_cast<const __FlashStringHelper *> (s)

	#define dstart(spd) Serial.begin (spd)
	#define debug(...) Serial.print (__VA_ARGS__)
	#define debugln(...) Serial.println (__VA_ARGS__)
#else
	#define dstart(...)
	#define debug(...)
	#define debugln(...)
#endif


// TODO: Move all the following tables to flash memory

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
	{Key::DEL,  Key::ENTER,    Key::POUND,     Key::HELP,  Key::F1,     Key::F2,    Key::F3,    Key::AT},
	{Key::_3,   Key::W,        Key::A,         Key::_4,    Key::Z,      Key::S,     Key::E,     Key::SHIFT},
	{Key::_5,   Key::R,        Key::D,         Key::_6,    Key::C,      Key::F,     Key::T,     Key::X},
	{Key::_7,   Key::Y,        Key::G,         Key::_8,    Key::B,      Key::H,     Key::U,     Key::V},
	{Key::_9,   Key::I,        Key::J,         Key::_0,    Key::M,      Key::K,     Key::O,     Key::N},
	{Key::DOWN, Key::P,        Key::L,         Key::UP,    Key::PERIOD, Key::COLON, Key::MINUS, Key::COMMA},
	{Key::LEFT, Key::ASTERISK, Key::SEMICOLON, Key::RIGHT, Key::ESC,    Key::EQUAL, Key::PLUS,  Key::SLASH},
	{Key::_1,   Key::CLEAR,    Key::CTRL,      Key::_2,    Key::SPACE,  Key::CMD,   Key::Q,     Key::RUNSTOP}
};

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

// Current state of the keyboard matris, true if a key is pressed
boolean matrix[MATRIX_ROWS][MATRIX_COLS] = {false};

struct LedCoordinates {
	byte row;
	byte col;
};

// Maps a key to the (row, col) tuple that controls its led.
LedCoordinates ledCoordinates[N_KEYS];

// Populates the ledCoordinates array
boolean buildLedCoordinates () {
	bool found = false;
	
	for (byte i = 0; i < N_KEYS; ++i) {
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
			const LedCoordinates& pos = ledCoordinates[static_cast<int> (k)];
			lc.setLed(0, pos.row, pos.col, true);
			break;
		}
		case Mode::PRESSED_OFF: {
			const Key k = keymap[row][col];
			const LedCoordinates& pos = ledCoordinates[static_cast<int> (k)];
			lc.setLed(0, pos.row, pos.col, false);
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
			const LedCoordinates& pos = ledCoordinates[static_cast<int> (k)];
			lc.setLed(0, pos.row, pos.col, false);
			break;
		}
		case Mode::PRESSED_OFF: {
			const Key k = keymap[row][col];
			const LedCoordinates& pos = ledCoordinates[static_cast<int> (k)];
			lc.setLed(0, pos.row, pos.col, true);
			break;
		}
		case Mode::ALWAYS_ON:
		case Mode::ALWAYS_OFF:
			// Nothing to do
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

	// Build the LED coordinates array
	if (!buildLedCoordinates()) {
		debugln (F("Unable to build the LED coordinates array, this indicatess a mistake in the code"));

		// Hang with fast blinking
		while (true) {
			digitalWrite (PIN_LED_R, HIGH);
			delay (222);
			digitalWrite (PIN_LED_R, LOW);
			delay (222);
		}
	}

	// Do the power-up animation
	for (byte i = 0; i < N_KEYS + 1; ++i) {
		const LedCoordinates& pos = ledCoordinates[static_cast<int> (splash_order[i])];
		lc.setLed (0, pos.row, pos.col, true);
		delay (DELAY_TIME);
		lc.setLed (0, pos.row, pos.col, false);
	}

	// Prepare the initial LED pattern according to the chosen mode
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
}

void loop () {
	while (matrixSamples.available ()) {
		KeyMatrixSample sample = matrixSamples.get ();

		if (sample.rows == 0x00 && sample.cols == 0xFF) {
			// All keys released
			for (byte r = 0; r < MATRIX_ROWS; ++r) {
				for (byte c = 0; c < MATRIX_COLS; ++c) {
					if (matrix[r][c]) {
						debug ("Released ");
						debug (r);
						debug (',');
						debug (c);
						debug (": ");
						debugln (KEY_NAMES[r][c]);

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
							debug ("Pressed ");
							debug (r);
							debug (',');
							debug (c);
							debug (": ");
							debugln (KEY_NAMES[r][c]);

							onKeyPressed (r, c);
						} else if (!pressed && matrix[r][c]) {
							debug ("Released ");
							debug (r);
							debug (',');
							debug (c);
							debug (": ");
							debugln (KEY_NAMES[r][c]);

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
