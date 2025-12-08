/**
 * ___  ___          _    ______                     _  __    ____
 * |  \/  |         | |   | ___ \                   | |/  |  / ___|
 * | .  . | ___  ___| |__ | |_/ / ___   __ _ _ __ __| |`| | / /___
 * | |\/| |/ _ \/ __| '_ \| ___ \/ _ \ / _` | '__/ _` | | | | ___ \
 * | |  | |  __/ (__| | | | |_/ / (_) | (_| | | | (_| |_| |_| \_/ |
 * \_|  |_/\___|\___|_| |_\____/ \___/ \__,_|_|  \__,_|\___/\_____/
 *
 * Copyright (c) 2024-2025 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <Arduino.h>

constexpr byte PIN_LED_R = A5;
constexpr byte PIN_LED_G = A4;
constexpr byte PIN_LED_B = A3;
constexpr byte PIN_MAX7221_SEL = A2;
constexpr byte PIN_MAX7221_DATA = A1;
constexpr byte PIN_MAX7221_CLK = A0;
constexpr byte PIN_RESET = 5;

// TODO: Make another KeyboardScanner with this stuff
/** \brief Number of rows in the C16/Plus4 keyboard matrix
 */
constexpr byte MATRIX_ROWS = 8;

/** \brief Number of columns in the C16/Plus4 keyboard matrix
 */
constexpr byte MATRIX_COLS = 8;

#include "LedControl.h"
LedControl lc (PIN_MAX7221_DATA, PIN_MAX7221_CLK, PIN_MAX7221_SEL, 1 /* Number of MAX72xx chips */);

//~ unsigned long DELAY_TIME = 35;

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

#ifdef ENABLE_LEDCONTROLLER_MAX7221
#include "LedControllerMax7221.h"
LedControllerMax7221 ledControllerMax7221;
LedController& ledController = ledControllerMax7221;
#elif defined(ENABLE_LEDCONTROLLER_NEOPIXEL)
#include "LedControllerNeoPixel.h"
LedControllerNeoPixel ledControllerNeoPixel;
LedController& ledController = ledControllerNeoPixel;
#endif

#include "AnimationChasing.h"
AnimationChasing animationChasing;

#include "AnimationScrollingColumn.h"
AnimationScrollingColumn animationScrollingColumn;

constexpr byte N_ANIMATIONS = 2;

Animation *animations[N_ANIMATIONS] = {
	&animationChasing,
	&animationScrollingColumn
};

#include "OCPin.h"
OpenCollectorPin<PIN_RESET> reset;
constexpr unsigned long RESET_LENGTH_MS = 200;

#include "ClockGenerator.h"
ClockGenerator clockGenerator;

#include "common.h"
#include "logo.h"
#include "C16Key.h"

enum class Mode: byte {
	ALWAYS_OFF,
	ALWAYS_ON,
	PRESSED_ON,
	PRESSED_OFF
};

//! \name Configuration values saved in EEPROM
//! @{
Mode mode = Mode::PRESSED_OFF;

byte animationId = 0;

byte brightness = MAX_BRIGHTNESS;

Clock clock = Clock::PAL;
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
constexpr word EEP_CLOCK = 0x103;

#include <avr/pgmspace.h>

#include "keymap.h"
#include "MatrixCoordinates.h"

KeyMap keyMap;

/** \brief Keyboard matrix status
 *
 * Basically contains the mapped SB keycode when a key is pressed, 0 otherwise (which is KEY_RESERVED so it should be
 * OK).
 */
Key matrix[MATRIX_ROWS][MATRIX_COLS];

// Called when a keypress is detected
void onKeyPressed (const C16Key k) {
	switch (mode) {
		case Mode::PRESSED_ON:
			ledController.setLedForKey (k, true);
			break;
		case Mode::PRESSED_OFF:
			ledController.setLedForKey (k, false);
			break;
		case Mode::ALWAYS_ON:
		case Mode::ALWAYS_OFF:
			// Nothing to do
			break;
	}
}

// Called when a keyrelease is detected
void onKeyReleased (const C16Key k) {
	switch (mode) {
		case Mode::PRESSED_ON:
			ledController.setLedForKey (k, false);
			break;
		case Mode::PRESSED_OFF:
			ledController.setLedForKey (k, true);
			break;
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
		const MatrixCoordinates pos = keyMap.getCoordinates (k);
		return matrix[pos.row][pos.col] != 0;
	}
}

void updateLighting () {
	// Update the LED pattern according to the chosen mode
	switch (mode) {
		case Mode::ALWAYS_ON:
			// Turn all leds on
			ledController.setAllLeds (true);
			break;
		case Mode::PRESSED_OFF:
			for (byte row = 0; row < MATRIX_ROWS; ++row) {
				for (byte col = 0; col < MATRIX_COLS; ++col) {
					const C16Key k = keyMap.getKey (row, col);
					ledController.setLedForKey (k, matrix[row][col] != 0 ? false : true);
				}
			}
			break;
		case Mode::ALWAYS_OFF:
			// Turn all leds off
			ledController.setAllLeds (false);
			break;
		case Mode::PRESSED_ON:
			for (byte row = 0; row < MATRIX_ROWS; ++row) {
				for (byte col = 0; col < MATRIX_COLS; ++col) {
					const C16Key k = keyMap.getKey (row, col);
					ledController.setLedForKey (k, matrix[row][col] != 0 ? true : false);
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
		ledController.setBrightness (brightness);
		Log.debug (F("Brightness set to %d\n"), static_cast<int> (brightness));
	}
}

void onReset () {
	reset.low ();
	delay (RESET_LENGTH_MS);
	reset.high ();
}

void onSetClock (const Clock newClock) {
	if (newClock != clock) {
		Log.debug (F("Setting clock %d\n"), static_cast<int> (newClock));

		clock = newClock;
		EEPROM.write (EEP_CLOCK, static_cast<byte> (clock));
		clockGenerator.setClock (newClock);
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
			if (usbKeycode != 0 && newBuf.find (usbKeycode, eventKeyCompare) < 0) {
				// Key released
				const C16Key k = keyMap.getKey (r, c);
				Log.debug (F("Key released: %s\n"), keyMap.getKeyName (k));
				Log.trace (F("USB Key released: %X\n"), (int) usbKeycode);
				onKeyReleased (k);
				boolean ok = usbKeyboard.release (usbKeycode);
#ifdef PEDANTIC_PRESS_RELEASE_CHECKS
				if (ok) {
#endif
					// Mark as released
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
			const C16Key k = keyMap.getKey (evt.row, evt.col);
			Log.debug (F("Key pressed: %s\n"), keyMap.getKeyName (k));
			Log.trace (F("USB Key pressed: %X\n"), (int) evt.key);
			onKeyPressed (k);
			boolean ok = usbKeyboard.press (evt.key);
#ifdef PEDANTIC_PRESS_RELEASE_CHECKS
			if (ok) {
#endif
				// Mark as pressed
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

void setup () {
#ifndef DISABLE_LOGGING
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

	byte c = EEPROM.read (EEP_CLOCK);
	if (c <= static_cast<byte> (Clock::NTSC)) {
		clock = static_cast<Clock> (c);
	} else {
		// Default clock
		clock = Clock::PAL;
	}
	clockGenerator.begin (clock);

	// R/G/B LED pins: configure as OUTPUTs and turn on
	pinMode (PIN_LED_R, OUTPUT);
	digitalWrite (PIN_LED_R, HIGH);
	pinMode (PIN_LED_G, OUTPUT);
	digitalWrite (PIN_LED_G, HIGH);
	pinMode (PIN_LED_B, OUTPUT);
	digitalWrite (PIN_LED_B, HIGH);

	/* Wake up and configure the led controller ASAP, since it might show a random pattern at startup, and build the
	 * required coordinates array
	 */
	if (!ledController.begin () || !keyMap.begin ()) {
		Log.error (F("Unable to build the LED coordinates array, this indicates a mistake in the code\n"));

		// Hang with fast blinking
		while (true) {
			digitalWrite (PIN_LED_R, HIGH);
			delay (222);
			digitalWrite (PIN_LED_R, LOW);
			delay (222);
		}
	}

	// Continue with led configuration
	brightness = EEPROM.read (EEP_BRIGHTNESS);
	if (brightness > MAX_BRIGHTNESS) {
		brightness = MAX_BRIGHTNESS;
	}
	ledController.setBrightness(brightness);


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
	animation.begin (ledController);
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
				lastCombo = C16Key::F1;
			} else if (isPressed (C16Key::F2)) {
				onSetMode (Mode::ALWAYS_ON);
				lastCombo = C16Key::F2;
			} else if (isPressed (C16Key::F3)) {
				onSetMode (Mode::PRESSED_ON);
				lastCombo = C16Key::F3;
			} else if (isPressed (C16Key::HELP)) {
				onSetMode (Mode::PRESSED_OFF);
				lastCombo = C16Key::HELP;
			} else if (isPressed (C16Key::_1)) {
				onSetAnimation (0);
				lastCombo = C16Key::_1;
			} else if (isPressed (C16Key::_2)) {
				onSetAnimation (1);
				lastCombo = C16Key::_2;
			} else if (isPressed (C16Key::PLUS)) {
				onSetBrightness (+1);
				lastCombo = C16Key::PLUS;
			} else if (isPressed (C16Key::MINUS)) {
				onSetBrightness (-1);
				lastCombo = C16Key::MINUS;
			} else if (isPressed (C16Key::P)) {
				onSetClock (Clock::PAL);
				lastCombo = C16Key::P;
			} else if (isPressed (C16Key::N)) {
				onSetClock (Clock::NTSC);
				lastCombo = C16Key::N;
			} else if (isPressed (C16Key::DEL)) {
				onReset ();
				lastCombo = C16Key::DEL;		// FIXME: HOLD UNTIL PRESSED?
			} else {
				lastCombo = C16Key::NONE;
			}
		}
	}

	//~ digitalWrite (PIN_LED_R, HIGH);
	//~ delay (1000);
	//~ digitalWrite (PIN_LED_R, LOW);
	//~ delay (1000);

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
