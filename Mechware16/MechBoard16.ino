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

// TODO: Make another KeyboardScanner with this stuff
/** \brief Number of rows in the C16/Plus4 keyboard matrix
 */
const byte MATRIX_ROWS = 8;

/** \brief Number of columns in the C16/Plus4 keyboard matrix
 */
const byte MATRIX_COLS = 8;

#include "LedControl.h"
LedControl lc (PIN_MAX7221_DATA, PIN_MAX7221_CLK, PIN_MAX7221_SEL, 1 /* Number of MAX72xx chips */);

unsigned long DELAY_TIME = 35;

#include "KbdScannerC16.h"
KbdScannerC16 kbdScannerC16;

#include "KbdScannerPassive16.h"
KbdScannerPassive16 kbdScannerPassive;

/** \brief Actual keyboard scanner in use
 *
 * Make sure this is NEVER NULL!
 */
KeyboardScanner *kbdScanner;

#include "UsbKeyboard.h"
UsbKeyboard usbKeyboard;

#include "AnimationChasing.h"
AnimationChasing animationChasing;

#include "AnimationScrollingColumn.h"
AnimationScrollingColumn animationScrollingColumn;

constexpr byte N_ANIMATIONS = 2;

Animation *animations[N_ANIMATIONS] = {
	&animationChasing,
	&animationScrollingColumn
};

#include "C16Key.h"
#include "logo.h"


enum class Mode: byte {
	ALWAYS_OFF,
	ALWAYS_ON,
	PRESSED_ON,
	PRESSED_OFF
};

// MAX72xx min/max brightness values
constexpr byte MIN_BRIGHTNESS = 0;
constexpr byte MAX_BRIGHTNESS = 15;

//! \name Configuration values saved in EEPROM
//! @{
Mode mode = Mode::PRESSED_OFF;

byte animationId = 0;

byte brightness = MAX_BRIGHTNESS;
//! @}

/*******************************************************************************
 * END OF SETTINGS
 ******************************************************************************/

#include "Log.h"
Logging Log;

#include <EEPROM.h>
constexpr word EEP_ANIMATION = 0x100;
constexpr word EEP_MODE = 0x101;
constexpr word EEP_BRIGHTNESS = 0x102;

#include <avr/pgmspace.h>

#include "MatrixCoordinates.h"

/* Maps a key to the (row, col) tuple that controls its led.
 * Built by buildLedCoordinates().
 */
MatrixCoordinates ledCoordinates[N_PHYSICAL_KEYS];

/* Maps a key to the (row, col) tuple describing its position in the keyboard matrix, useful for quick lookups.
 * Built by buildKeyCoordinates().
 */
MatrixCoordinates keyCoordinates[N_PHYSICAL_KEYS];

/** \brief Keyboard matrix status
 *
 * Basically contains the mapped SB keycode when a key is pressed, 0 otherwise (which is KEY_RESERVED so it should be
 * OK).
 */
Key matrix[MATRIX_ROWS][MATRIX_COLS];

constexpr C16Key keymap[MATRIX_ROWS][MATRIX_COLS] PROGMEM = {
	{C16Key::DEL,  C16Key::RETURN,   C16Key::POUND,     C16Key::HELP,  C16Key::F1,     C16Key::F2,    C16Key::F3,    C16Key::AT},
	{C16Key::_3,   C16Key::W,        C16Key::A,         C16Key::_4,    C16Key::Z,      C16Key::S,     C16Key::E,     C16Key::SHIFT},
	{C16Key::_5,   C16Key::R,        C16Key::D,         C16Key::_6,    C16Key::C,      C16Key::F,     C16Key::T,     C16Key::X},
	{C16Key::_7,   C16Key::Y,        C16Key::G,         C16Key::_8,    C16Key::B,      C16Key::H,     C16Key::U,     C16Key::V},
	{C16Key::_9,   C16Key::I,        C16Key::J,         C16Key::_0,    C16Key::M,      C16Key::K,     C16Key::O,     C16Key::N},
	{C16Key::DOWN, C16Key::P,        C16Key::L,         C16Key::UP,    C16Key::PERIOD, C16Key::COLON, C16Key::MINUS, C16Key::COMMA},
	{C16Key::LEFT, C16Key::ASTERISK, C16Key::SEMICOLON, C16Key::RIGHT, C16Key::ESC,    C16Key::EQUAL, C16Key::PLUS,  C16Key::SLASH},
	{C16Key::_1,   C16Key::CLEAR,    C16Key::CTRL,      C16Key::_2,    C16Key::SPACE,  C16Key::CMD,   C16Key::Q,     C16Key::RUNSTOP}
};

// Only useful for debugging
//~ constexpr char KEY_NAMES[MATRIX_ROWS][MATRIX_COLS][4] = {
	//~ {"DEL", "RET", "£",   "HLP", "F1",  "F2", "F3", "@"},
	//~ {"3",   "W",   "A",   "4",   "Z",   "S",  "E",  "SHF"},
	//~ {"5",   "R",   "D",   "6",   "C",   "F",  "T",  "X"},
	//~ {"7",   "Y",   "G",   "8",   "B",   "H",  "U",  "V"},
	//~ {"9",   "I",   "J",   "0",   "M",   "K",  "O",  "N"},
	//~ {"DN",  "P",   "L",   "UP",  ".",   ":",  "-",  ","},
	//~ {"LF",  "*",   ";",   "RT",  "ESC", "=",  "+",  "/"},
	//~ {"1",   "CLR", "CTL", "2",   "SPC", "C=", "Q",  "RUN"}
//~ };

// Populates the ledCoordinates array
boolean buildLedCoordinates () {
	bool found;
	
	for (byte i = 0; i < N_PHYSICAL_KEYS; ++i) {
		found = false;
		for (byte row = 0; row < MATRIX_ROWS && !found; ++row) {
			for (byte col = 0; col < MATRIX_COLS && !found; ++col) {
				if (pgm_read_byte (&keymap[row][col]) == i) {
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
		found = false;
		for (byte row = 0; row < MATRIX_ROWS && !found; ++row) {
			for (byte col = 0; col < MATRIX_COLS && !found; ++col) {
				if (pgm_read_byte (&keymap[row][col]) == i) {
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

// Called when a keypress is detected
void onKeyPressed (const byte row, const byte col) {
	switch (mode) {
		case Mode::PRESSED_ON: {
			const byte k = pgm_read_byte (&keymap[row][col]);
			const MatrixCoordinates& pos = ledCoordinates[k];
			lc.setLed (0, pos.row, pos.col, true);
			break;
		}
		case Mode::PRESSED_OFF: {
			const byte k = pgm_read_byte (&keymap[row][col]);
			const MatrixCoordinates& pos = ledCoordinates[k];
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
			const byte k = pgm_read_byte (&keymap[row][col]);
			const MatrixCoordinates& pos = ledCoordinates[k];
			lc.setLed (0, pos.row, pos.col, false);
			break;
		}
		case Mode::PRESSED_OFF: {
			const byte k = pgm_read_byte (&keymap[row][col]);
			const MatrixCoordinates& pos = ledCoordinates[k];
			lc.setLed (0, pos.row, pos.col, true);
			break;
		}
		case Mode::ALWAYS_ON:
		case Mode::ALWAYS_OFF:
			// Nothing to do
			break;
	}
}

boolean isPressed (const C16Key k) {
	if (static_cast<byte> (k) > N_PHYSICAL_KEYS) {
		return false;
	} else {
		const MatrixCoordinates pos = keyCoordinates[static_cast<byte> (k)];
		return matrix[pos.row][pos.col] != 0;
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
					const byte k = pgm_read_byte (&keymap[r][c]);
					const MatrixCoordinates& pos = ledCoordinates[k];
					lc.setLed (0, pos.row, pos.col, matrix[r][c] != 0 ? false : true);
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
					const byte k = pgm_read_byte (&keymap[r][c]);
					const MatrixCoordinates& pos = ledCoordinates[k];
					lc.setLed (0, pos.row, pos.col, matrix[r][c] != 0 ? true : false);
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
	if (newAnimation != animationId) {
		Log.debug (F("Setting animation %d\n"), static_cast<int> (newAnimation));
		
		animationId = newAnimation;
		EEPROM.write (EEP_ANIMATION, static_cast<byte> (animationId));
	}
}

void onSetBrightness (const int8_t diff) {
	int newBrightness = brightness + diff;
	if (newBrightness >= MIN_BRIGHTNESS && newBrightness <= MAX_BRIGHTNESS) {
		brightness = static_cast<byte> (newBrightness);
		EEPROM.write (EEP_BRIGHTNESS, static_cast<byte> (brightness));
		lc.setIntensity (0, brightness);
		Log.debug (F("Brightness set to %d\n"), static_cast<int> (brightness));
	}
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
	Log.info (PSTR_TO_F (logo));
	Log.info (F("---------------------------------------- Version " MECH16_VERSION_STR " ---------\n"));
	Log.setShowLevel (true);

	Log.info (F("Built on %s %s\n"), __DATE__, __TIME__);
	
	// Wake up and configure the MAX72XX ASAP, since it might show a random pattern at startup
	lc.shutdown (0, false);
	lc.clearDisplay (0);
	brightness = EEPROM.read (EEP_BRIGHTNESS);
	if (brightness > MAX_BRIGHTNESS) {
		brightness = MAX_BRIGHTNESS;
	}
	lc.setIntensity (0, brightness);

	// R/G/B LED pins: configure as OUTPUTs and turn on
	pinMode (PIN_LED_R, OUTPUT);
	digitalWrite (PIN_LED_R, HIGH);
	pinMode (PIN_LED_G, OUTPUT);
	digitalWrite (PIN_LED_G, HIGH);
	pinMode (PIN_LED_B, OUTPUT);
	digitalWrite (PIN_LED_B, HIGH);

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

	// Start with normal keyboard scanner...
	kbdScanner = &kbdScannerC16;
	DDRB  = 0x00;   // Output port: all inputs...
	PORTB = 0xFF;   // ... with pull-ups
	DDRD = 0x00;	// Input port too, just in case some key is being held at startup
	PORTD = 0xFF;

	// ... and while we play the power-up animation...
	animationId = EEPROM.read (EEP_ANIMATION);
	if (animationId >= N_ANIMATIONS) {
		// Default animation
		animationId = 0;
	}

	Log.debug (F("Playing intro animation %d\n"), static_cast<int> (animationId));
	Animation& animation = *animations[animationId];
	animation.begin (lc);
	unsigned long start = millis ();
	while (animation.step ()) {
		// ... check if we have activity on PINB (our wannabe-output port) ...
		if (PINB != 0xFF) {
			// ... and, if we do, switch to the passive scanner
			Log.info (F("Using PASSIVE scanner\n"));
			kbdScanner = &kbdScannerPassive;
		}
	}

	// If animation takes less than 200 ms, wait for another bit, just in case
	while ((kbdScanner != &kbdScannerPassive) && (millis () - start < 200UL)) {
		if (PINB != 0xFF) {
			// Detected activity on the wannabe-output port, switch to the passive scanner
			Log.info (F("Using PASSIVE scanner\n"));
			kbdScanner = &kbdScannerPassive;
			break;
		}
	}
	Log.debug (F("Animation done\n"));

	// Prepare the initial LED pattern according to the saved mode
	byte b = EEPROM.read (EEP_MODE);
	if (b <= static_cast<byte> (Mode::PRESSED_OFF)) {
		mode = static_cast<Mode> (b);
	} else {
		// Default mode
		mode = Mode::PRESSED_OFF;
	}
	updateLighting ();

	if (kbdScanner -> begin ()) {
		// Clear matrix
		for (byte r = 0; r < MATRIX_ROWS; ++r) {
			for (byte c = 0; c < MATRIX_COLS; ++c) {
				matrix[r][c] = 0;
			}
		}
	} else {
		Log.error (F("Failed to initialize keyboard scanner\n"));
	}

	usbKeyboard.begin ();
}

void loop () {
	static C16Key lastCombo = C16Key::NONE;
	
	// Check combos
	if (isPressed (C16Key::CMD) && isPressed (C16Key::CTRL)) {
		if (lastCombo == C16Key::NONE || !isPressed (lastCombo)) {		// Poor way to avoid key repetitions
			if (isPressed (C16Key::F1)) {
				onSetMode (Mode::ALWAYS_OFF);
			} else if (isPressed (C16Key::F2)) {
				onSetMode (Mode::ALWAYS_ON);
			} else if (isPressed (C16Key::F3)) {
				onSetMode (Mode::PRESSED_ON);
			} else if (isPressed (C16Key::HELP)) {
				onSetMode (Mode::PRESSED_OFF);
			} else if (isPressed (C16Key::_1)) {
				onSetAnimation (0);						
			} else if (isPressed (C16Key::_2)) {
				onSetAnimation (1);
			} else if (isPressed (C16Key::PLUS)) {
				onSetBrightness (+1);
				lastCombo = C16Key::PLUS;
			} else if (isPressed (C16Key::MINUS)) {
				onSetBrightness (-1);
				lastCombo = C16Key::MINUS;
			} else if (isPressed (C16Key::RUNSTOP)) {
				// TODO: RESET
			} else {
				lastCombo = C16Key::NONE;
			}
		}
	}

	static unsigned long lastKeyboardScanTime = 0;
	
	// Let the scanner do its own housekeeping as often as possible
	kbdScanner -> loop ();

	// Once in a while, do the scanning
	if (millis () - lastKeyboardScanTime >= KEYBOARD_SCAN_INTERVAL_MS) {
		KeyBuffer kBuf;
		kBuf.begin ();
		KeyboardScanner::ScanStatus scanStatus = kbdScanner -> scan (kBuf);
		if (scanStatus == KeyboardScanner::SCAN_COMPLETE) {
			handleKeyboard (kBuf);
		}

		// Update leds - Note that this needs a patched Keyboard library
		byte leds = usbKeyboard.getLeds ();
		kbdScanner -> updateLeds (
			leds & USBLED_CAPS_LOCK,
			leds & USBLED_NUM_LOCK,
			leds & USBLED_SCROLL_LOCK
		);
			
		lastKeyboardScanTime = millis ();
	}
}

/** \brief Updates matrix and generates key press/release events
 *
 * \param newBuf Keys currently being pressed
 */
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
	for (byte r = 0; r < MATRIX_ROWS; ++r) {
		for (byte c = 0; c < MATRIX_COLS; ++c) {
			Key& usbKeycode = matrix[r][c];
			if (newBuf.find (usbKeycode, eventKeyCompare) < 0) {
				// Key released
				Log.trace (F("USB Key released: %X\n"), (int) usbKeycode);
				onKeyReleased (r, c);			// Call this now, before we alter i
				boolean ok = usbKeyboard.release (usbKeycode);
#ifdef PEDANTIC_PRESS_RELEASE_CHECKS
				if (ok) {
#endif
					usbKeycode = 0;		// It's a reference so this works :)
#ifdef PEDANTIC_PRESS_RELEASE_CHECKS
				} else {
#else
				if (!ok) {
#endif
					Log.error (F("Key release failed: %X\n"), (int) usbKeycode);
				}
			}
		}
	}
	
	// Check for keys that were just pressed
	for (byte i = 0; i < newBuf.size; ++i) {
		const KeyEvent& evt = newBuf[i];
		if (matrix[evt.row][evt.col] != evt.key) {
			// New key pressed
			Log.trace (F("USB Key pressed: %X\n"), (int) evt.key);
			onKeyPressed (evt.row, evt.col);
			boolean ok = usbKeyboard.press (evt.key);
#ifdef PEDANTIC_PRESS_RELEASE_CHECKS
			if (ok) {
#endif
				matrix[evt.row][evt.col] = evt.key;
#ifdef PEDANTIC_PRESS_RELEASE_CHECKS
			} else {
#else
			if (!ok) {
#endif
				/* Any failures are probably due to the internal HID Library
				 * buffer being full
				 */
				Log.error (F("Key press failed: %X\n"), (int) evt.key);
			}
		}
	}

	usbKeyboard.commit ();
}
