/**
 * ___  ___          _    ______                     _  __    ____
 * |  \/  |         | |   | ___ \                   | |/  |  / ___|
 * | .  . | ___  ___| |__ | |_/ / ___   __ _ _ __ __| |`| | / /___
 * | |\/| |/ _ \/ __| '_ \| ___ \/ _ \ / _` | '__/ _` | | | | ___ \
 * | |  | |  __/ (__| | | | |_/ / (_) | (_| | | | (_| |_| |_| \_/ |
 * \_|  |_/\___|\___|_| |_\____/ \___/ \__,_|_|  \__,_|\___/\_____/
* 
 * Copyright (c) 2024 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

const byte PIN_LED_R = A5;
const byte PIN_LED_G = A4;
const byte PIN_LED_B = A3;
const byte PIN_MAX7221_SEL = A2;
const byte PIN_MAX7221_DATA = A1;
const byte PIN_MAX7221_CLK = A0;

// TODO: Make a nother KeyboardScanner with this stuff
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

#include "KbdScannerC16.h"
KbdScannerC16 kbdScanner;

#include "SmallBuffer.h"
SmallBuffer<word, 16> keyBuffer;

#include "UsbKeyboard.h"
UsbKeyboard usbKeyboard;

#include "logo.h"

enum class Key2: byte {
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

const byte N_PHYSICAL_KEYS = static_cast<byte> (Key2::SPACE) + 1;

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

#include "Log.h"
Logging Log;

#include <EEPROM.h>
constexpr word EEP_ANIMATION = 0x100;
constexpr word EEP_MODE = 0x101;

#include <avr/pgmspace.h>

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
const Key2 splash0row4[splash0nCols] PROGMEM = {Key2::NONE,    Key2::ESC,   Key2::_1,   Key2::_2,    Key2::_3,   Key2::_4,   Key2::_5,    Key2::_6,   Key2::_7,   Key2::_8,    Key2::_9,     Key2::_0,    Key2::LEFT,      Key2::RIGHT,    Key2::UP,     Key2::DOWN,   Key2::DEL,   Key2::F1};
const Key2 splash0row3[splash0nCols] PROGMEM = {Key2::NONE,    Key2::CTRL,  Key2::Q,    Key2::W,     Key2::E,    Key2::R,    Key2::T,     Key2::Y,    Key2::U,    Key2::I,     Key2::O,      Key2::P,     Key2::AT,        Key2::PLUS,     Key2::MINUS,  Key2::NONE,   Key2::CLEAR, Key2::F2};
const Key2 splash0row2[splash0nCols] PROGMEM = {Key2::RUNSTOP, Key2::NONE,  Key2::A,    Key2::S,     Key2::D,    Key2::F,    Key2::G,     Key2::H,    Key2::J,    Key2::K,     Key2::L,      Key2::COLON, Key2::SEMICOLON, Key2::ASTERISK, Key2::NONE,   Key2::RETURN, Key2::NONE,  Key2::F3};
const Key2 splash0row1[splash0nCols] PROGMEM = {Key2::CMD,     Key2::SHIFT, Key2::Z,    Key2::X,     Key2::C,    Key2::V,    Key2::B,     Key2::N,    Key2::M,    Key2::COMMA, Key2::PERIOD, Key2::SLASH, Key2::NONE,      Key2::SHIFT,    Key2::POUND,  Key2::EQUAL,  Key2::NONE,  Key2::HELP};
const Key2 splash0row0[splash0nCols] PROGMEM = {Key2::NONE,    Key2::NONE,  Key2::NONE, Key2::SPACE, Key2::NONE, Key2::NONE, Key2::SPACE, Key2::NONE, Key2::NONE, Key2::SPACE, Key2::NONE,   Key2::NONE,  Key2::NONE,      Key2::NONE,     Key2::NONE,   Key2::NONE,   Key2::NONE,  Key2::NONE};

constexpr Key2 const * splash0rows[N_PHYSICAL_ROWS] PROGMEM = {splash0row4, splash0row3, splash0row2, splash0row1, splash0row0};

void splash0 () {
	for (byte i = 0; i < splash0nCols; ++i) {
		// Turn on a column...
		for (byte j = 0; j < N_PHYSICAL_ROWS; ++j) {
			const byte* krow = pgm_read_byte (&splash0rows[j]);
			const byte k = pgm_read_byte (&(krow[i]));
			if (static_cast<Key2> (k) != Key2::NONE) {
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
			if (static_cast<Key2> (k) != Key2::NONE) {
				const MatrixCoordinates& pos = ledCoordinates[k];
				lc.setLed (0, pos.row, pos.col, false);
			}
		}
	}
}

// Order in which keys appear on the keyboard (+1 for 2 Shifts)
constexpr Key2 splash_order[N_PHYSICAL_KEYS + 1] PROGMEM = {
	Key2::ESC, Key2::_1, Key2::_2, Key2::_3, Key2::_4, Key2::_5, Key2::_6, Key2::_7, Key2::_8, Key2::_9, Key2::_0, Key2::LEFT, Key2::RIGHT, Key2::UP, Key2::DOWN, Key2::DEL,
	Key2::CTRL, Key2::Q, Key2::W, Key2::E, Key2::R, Key2::T, Key2::Y, Key2::U, Key2::I, Key2::O, Key2::P, Key2::AT, Key2::PLUS, Key2::MINUS, Key2::CLEAR,
	Key2::RUNSTOP, /* Shift Lock */ Key2::A, Key2::S, Key2::D, Key2::F, Key2::G, Key2::H, Key2::J, Key2::K, Key2::L, Key2::COLON, Key2::SEMICOLON, Key2::ASTERISK, Key2::RETURN,
	Key2::CMD, Key2::SHIFT, Key2::Z, Key2::X, Key2::C, Key2::V, Key2::B, Key2::N, Key2::M, Key2::COMMA, Key2::PERIOD, Key2::SLASH, Key2::SHIFT, Key2::POUND, Key2::EQUAL,
	Key2::SPACE,
	Key2::HELP, Key2::F3, Key2::F2, Key2::F1    // Reverse order just to be cool ;)
};
	
void splash1 () {
	for (byte i = 0; i < N_PHYSICAL_KEYS + 1; ++i) {
		for (byte j = 0; j < 3; ++j) {
			const byte k = pgm_read_byte (&(splash_order[i + j]));
			const MatrixCoordinates& pos = ledCoordinates[k];
			lc.setLed (0, pos.row, pos.col, true);
		}
		delay (40);
		const byte k = pgm_read_byte (&(splash_order[i]));
		const MatrixCoordinates& pos = ledCoordinates[k];
		lc.setLed (0, pos.row, pos.col, false);
	}
}

constexpr Key2 keymap[MATRIX_ROWS][MATRIX_COLS] = {
	{Key2::DEL,  Key2::RETURN,   Key2::POUND,     Key2::HELP,  Key2::F1,     Key2::F2,    Key2::F3,    Key2::AT},
	{Key2::_3,   Key2::W,        Key2::A,         Key2::_4,    Key2::Z,      Key2::S,     Key2::E,     Key2::SHIFT},
	{Key2::_5,   Key2::R,        Key2::D,         Key2::_6,    Key2::C,      Key2::F,     Key2::T,     Key2::X},
	{Key2::_7,   Key2::Y,        Key2::G,         Key2::_8,    Key2::B,      Key2::H,     Key2::U,     Key2::V},
	{Key2::_9,   Key2::I,        Key2::J,         Key2::_0,    Key2::M,      Key2::K,     Key2::O,     Key2::N},
	{Key2::DOWN, Key2::P,        Key2::L,         Key2::UP,    Key2::PERIOD, Key2::COLON, Key2::MINUS, Key2::COMMA},
	{Key2::LEFT, Key2::ASTERISK, Key2::SEMICOLON, Key2::RIGHT, Key2::ESC,    Key2::EQUAL, Key2::PLUS,  Key2::SLASH},
	{Key2::_1,   Key2::CLEAR,    Key2::CTRL,      Key2::_2,    Key2::SPACE,  Key2::CMD,   Key2::Q,     Key2::RUNSTOP}
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
		Key2 k = static_cast<Key2> (i);

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
		Key2 k = static_cast<Key2> (i);

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
			const Key2 k = keymap[row][col];
			const MatrixCoordinates& pos = ledCoordinates[static_cast<int> (k)];
			lc.setLed (0, pos.row, pos.col, true);
			break;
		}
		case Mode::PRESSED_OFF: {
			const Key2 k = keymap[row][col];
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
			const Key2 k = keymap[row][col];
			const MatrixCoordinates& pos = ledCoordinates[static_cast<int> (k)];
			lc.setLed (0, pos.row, pos.col, false);
			break;
		}
		case Mode::PRESSED_OFF: {
			const Key2 k = keymap[row][col];
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
					const Key2 k = keymap[r][c];
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
					const Key2 k = keymap[r][c];
					const MatrixCoordinates& pos = ledCoordinates[static_cast<int> (k)];
					lc.setLed (0, pos.row, pos.col, keyboardMatrix[r][c]);
				}
			}
			break;
	}
}

void onSetMode (const Mode newMode) {
	if (newMode != mode) {
		Log.debug (F("Setting mode %d\n"), static_cast<int> (newMode));
		
		mode = newMode;
		EEPROM.write (EEP_MODE, static_cast<byte> (mode));
		updateLighting ();
	}
}

void onSetAnimation (const int newAnimation) {
	if (newAnimation != animation) {
		Log.debug (F("Setting animation %d\n"), static_cast<int> (newAnimation));
		
		animation = newAnimation;
		EEPROM.write (EEP_ANIMATION, static_cast<byte> (animation));
	}
}

void doAnimation () {
	Log.debug (F("Playing intro animation %d\n"), static_cast<int> (animation));
		
	switch (animation) {
		case 0:
		default:
			splash0 ();
			break;
		case 1:
			splash1 ();
			break;
	}

	Log.debug (F("Animation done\n"));
}

void setup () {
#if !defined (DISABLE_LOGGING) || defined (ENABLE_SERIAL_COMMANDS)
	Serial.begin (115200);
#ifdef ENABLE_DEVELOPER_MODE
	while (!Serial)
		;
#endif
#endif
#ifdef ENABLE_DEVELOPER_MODE
	Log.begin (LOG_LEVEL_DEBUG, &Serial);
#else
	Log.begin (LOG_LEVEL_INFO, &Serial);
#endif

	Log.setShowLevel (false);
	Log.info (logo);
	Log.info (F("Version " MECH16_VERSION_STR "\n"));
	Log.setShowLevel (true);
	
	// Wake up and configure the MAX72XX ASAP, since it might show a random pattern at startup
	lc.shutdown (0, false);
	lc.setIntensity (0, 15);		// 0-15
	lc.clearDisplay (0);

	/* Keyboard polling: TED drives the rows, which we have on PORT B, while the keyboard "outputs" the columns, which
	 * we have on PORT D. They are all INPUTs by default se all we have to do is to enable the pin-change interrupts on
	 * all pins of PORT B.
	 */
	//~ PCMSK0 = (1 << PCINT7) | (1 << PCINT6) | (1 << PCINT5) | (1 << PCINT4) |
	         //~ (1 << PCINT3) | (1 << PCINT2) | (1 << PCINT1) | (1 << PCINT0);
	//~ PCICR |= (1 << PCIE0);

	// R/G/B LED pins: configure as OUTPUTs
	pinMode (PIN_LED_R, OUTPUT);
	pinMode (PIN_LED_G, OUTPUT);
	pinMode (PIN_LED_B, OUTPUT);

	// Build the required coordinates array
	if (!buildLedCoordinates () || !buildKeyCoordinates ()) {
		Log.error (F("Unable to build the LED coordinates array, this indicates a mistake in the code\n"));

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

	if (kbdScanner.begin ()) {
		keyBuffer.begin ();
	} else {
		Log.error (F("Failed to initialize keyboard scanner\n"));
	}

	usbKeyboard.begin ();
}

boolean isPressed (const Key k) {
	const MatrixCoordinates pos = keyCoordinates[static_cast<byte> (k)];	// FIXME: Check bounds
	return keyboardMatrix[pos.row][pos.col];
}

void loop () {
	//~ while (matrixSamples.available ()) {
		//~ KeyMatrixSample sample = matrixSamples.get ();

		//~ /* When scanning the keyboard, the C16/+4 KERNAL first does a quick test to check if any key is pressed at all:
		 //~ * it brings all the rows down and checks whether all cols are up or not. If at least one column is down, it
		 //~ * goes on to check every individual row, as at least one key must be pressed and there's no other way to find
		 //~ * out exactly which one, otherwise it takes no further action and just terminates the scan there.
		 //~ */
		//~ if (sample.rows == 0x00 && sample.cols == 0xFF) {
			//~ // All keys released
			//~ for (byte r = 0; r < MATRIX_ROWS; ++r) {
				//~ for (byte c = 0; c < MATRIX_COLS; ++c) {
					//~ if (keyboardMatrix[r][c]) {
						//~ debug (F("Released "));
						//~ debug (r);
						//~ debug (F(","));
						//~ debug (c);
						//~ debug (F(": "));
						//~ debugln (KEY_NAMES[r][c]);

						//~ onKeyReleased (r, c);

						//~ keyboardMatrix[r][c] = false;
					//~ }
				//~ }
			//~ }
		//~ } else if (countSetBits (sample.rows) == 7) {
			//~ // Exactly one row is cleared, find out which one and update all its columns
			//~ for (byte r = 0; r < MATRIX_ROWS; ++r) {
				//~ if ((sample.rows & (1 << r)) == 0) {
					//~ for (byte c = 0; c < MATRIX_COLS; ++c) {
						//~ boolean pressed = (sample.cols & (1 << c)) == 0;
						//~ if (pressed && !keyboardMatrix[r][c]) {
							//~ debug (F("Pressed "));
							//~ debug (r);
							//~ debug (',');
							//~ debug (c);
							//~ debug (F(": "));
							//~ debugln (KEY_NAMES[r][c]);

							//~ onKeyPressed (r, c);
						//~ } else if (!pressed && keyboardMatrix[r][c]) {
							//~ debug (F("Released "));
							//~ debug (r);
							//~ debug (F(","));
							//~ debug (c);
							//~ debug (F(": "));
							//~ debugln (KEY_NAMES[r][c]);

							//~ onKeyReleased (r, c);
						//~ }
						//~ keyboardMatrix[r][c] = pressed;
					//~ }

					//~ break;		// There is necessarily only one row at 0
				//~ }
			//~ }
		//~ }
	//~ }

	//~ // Check combos
	//~ if (isPressed (Key2::CMD) && isPressed (Key2::CTRL)) {
		//~ if (isPressed (Key2::F1)) {
			//~ onSetMode (Mode::ALWAYS_OFF);
		//~ } else if (isPressed (Key2::F2)) {
			//~ onSetMode (Mode::ALWAYS_ON);
		//~ } else if (isPressed (Key2::F3)) {
			//~ onSetMode (Mode::PRESSED_ON);
		//~ } else if (isPressed (Key2::HELP)) {
			//~ onSetMode (Mode::PRESSED_OFF);
		//~ } else if (isPressed (Key2::_1)) {
			//~ onSetAnimation (0);
		//~ } else if (isPressed (Key2::_2)) {
			//~ onSetAnimation (1);
		//~ } else if (isPressed (Key2::RUNSTOP)) {
			//~ // TODO: RESET
		//~ }
	//~ }
	
	//~ digitalWrite (PIN_LED_R, HIGH);
	//~ delay (1000);
	//~ digitalWrite (PIN_LED_R, LOW);
	//~ delay (1000);

	static unsigned long lastKeyboardScanTime = 0;
	
	// Let the scanner do its own housekeeping as often as possible
	kbdScanner.loop ();

	// Once in a while, do the scanning
	if (millis () - lastKeyboardScanTime >= KEYBOARD_SCAN_INTERVAL_MS) {
		//~ unsigned long now = micros ();
		KeyBuffer kBuf;
		kBuf.begin ();
		KeyboardScanner::ScanStatus scanStatus = kbdScanner.scan (kBuf);
	
		//~ unsigned long dur = micros () - now;
		//~ debugln (dur);
		if (scanStatus == KeyboardScanner::SCAN_COMPLETE) {
			handleKeyboard (kBuf);
		}

		//~ // Update leds - Note that this needs a patched Keyboard library
		//~ byte leds = usbKeyboard.getLeds ();
		//~ kbdScanner.updateLeds (
			//~ leds & USBLED_CAPS_LOCK,
			//~ leds & USBLED_NUM_LOCK,
			//~ leds & USBLED_SCROLL_LOCK
		//~ );
			
		lastKeyboardScanTime = millis ();
	}
}

void handleKeyboard (const KeyBuffer& newBuf) {
	//~ if (newBuf.size > 0) {
		//~ debug (F("Handling new buffer: "));
		//~ for (byte i = 0; i < newBuf.size; ++i) {
			//~ debug (newBuf[i] & 0xFF, HEX);
			//~ debug (' ');
		//~ }
		//~ debugln (' ');
	//~ }

	// Check for keys that were just released
	for (byte i = 0; i < keyBuffer.size; ++i) {
		if (newBuf.find (keyBuffer[i]) < 0) {
			// Key released
			Log.trace (F("USB Key released: %X\n"), (int) keyBuffer[i]);
			boolean ok = usbKeyboard.release (keyBuffer[i]);
#ifdef PEDANTIC_PRESS_RELEASE_CHECKS
			if (ok) {
#endif
				if (!keyBuffer.remove (keyBuffer[i])) {
					Log.error (F("Remove from keybuffer failed\n"));
				} else {
					/* OK, this is crap but it works: basically we have changed the
					 * array we are iterating on, so we'd better restart from
					 * scratch
					 *
					 * (The for loop will make this 0 for the next cycle.)
					 */
					i = -1;
				}
#ifdef PEDANTIC_PRESS_RELEASE_CHECKS
			} else {
#else
			if (!ok) {
#endif
				Log.error (F("Key release failed: %X\n"), (int) keyBuffer[i]);
			}
		}
	}
	
	// Check for keys that were just pressed
	for (byte i = 0; i < newBuf.size; ++i) {
		if (keyBuffer.find (newBuf[i]) < 0) {
			// New key pressed
			Log.trace (F("USB Key pressed: %X\n"), (int) newBuf[i]);
			boolean ok = usbKeyboard.press (newBuf[i]);
#ifdef PEDANTIC_PRESS_RELEASE_CHECKS
			if (ok) {
#endif
				keyBuffer.append (newBuf[i]);
#ifdef PEDANTIC_PRESS_RELEASE_CHECKS
			} else {
#else
			if (!ok) {
#endif
				/* Any failures are probably due to the internal HID Library
				 * buffer being full
				 */
				Log.error (F("Key press failed: %X\n"), (int) keyBuffer[i]);
			}
		}
	}

	usbKeyboard.commit ();
}
