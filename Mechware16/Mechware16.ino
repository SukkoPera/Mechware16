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

#include "common.h"
#include <Arduino.h>
#include <DigitalIO.h>
#include <SoftPWM.h>

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

Animation *animations[] = {
	&animationChasing,
	&animationScrollingColumn
};

constexpr byte N_ANIMATIONS = sizeof (animations) / sizeof(animations[0]);

#include "OCPin.h"
OpenCollectorPin<PIN_RESET> resetOutput;
constexpr unsigned long RESET_LENGTH_MS = 200;

#include "ClockGenerator.h"
ClockGenerator clockGenerator;

#include "common.h"
#include "logo.h"
#include "C16Key.h"

#include "userconfig.h"
constexpr byte MACHINE_SETTINGS_NO = sizeof (machineSettings) / sizeof(machineSettings[0]);
constexpr byte HOTKEYS_NO = sizeof (hotKeys) / sizeof(hotKeys[0]);

//! \name Configuration values saved in EEPROM
//! @{
LightingMode mode = LightingMode::PRESSED_OFF;

byte animationId = 0;

byte brightness = MAX_BRIGHTNESS;

// Index in machineSettings[]
byte configuration = -1;
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
constexpr word EEP_CONFIGURATION = 0x103;

#include <avr/pgmspace.h>

#include "keymap.h"
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
		case LightingMode::PRESSED_ON:
			ledController.setLedForKey (k, true);
			break;
		case LightingMode::PRESSED_OFF:
			ledController.setLedForKey (k, false);
			break;
		case LightingMode::ALWAYS_ON:
		case LightingMode::ALWAYS_OFF:
			// Nothing to do
			break;
	}
}

// Called when a keyrelease is detected
void onKeyReleased (const C16Key k) {
	switch (mode) {
		case LightingMode::PRESSED_ON:
			ledController.setLedForKey (k, false);
			break;
		case LightingMode::PRESSED_OFF:
			ledController.setLedForKey (k, true);
			break;
		case LightingMode::ALWAYS_ON:
		case LightingMode::ALWAYS_OFF:
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
		case LightingMode::ALWAYS_ON:
			// Turn all leds on
			ledController.setAllLeds (true);
			break;
		case LightingMode::PRESSED_OFF:
			for (byte row = 0; row < MATRIX_ROWS; ++row) {
				for (byte col = 0; col < MATRIX_COLS; ++col) {
					const C16Key k = keyMap.getKey (row, col);
					ledController.setLedForKey (k,isPressed (k) ? false : true);
					// ledController.setLedForKey (k, matrix[row][col] != 0 ? false : true);
				}
			}
			break;
		case LightingMode::ALWAYS_OFF:
			// Turn all leds off
			ledController.setAllLeds (false);
			break;
		case LightingMode::PRESSED_ON:
			for (byte row = 0; row < MATRIX_ROWS; ++row) {
				for (byte col = 0; col < MATRIX_COLS; ++col) {
					const C16Key k = keyMap.getKey (row, col);
					ledController.setLedForKey (k, isPressed (k) ? true : false);
					// ledController.setLedForKey (k, matrix[row][col] != 0 ? true : false);
				}
			}
			break;
	}
}

void setLightingMode (const LightingMode newMode) {
	if (newMode != mode) {
		Log.info (F("Setting mode %d\n"), static_cast<int> (newMode));

		mode = newMode;
		EEPROM.write (EEP_MODE, static_cast<byte> (mode));
		updateLighting ();
	}
}

void setAnimation (const int newAnimation) {
	if (newAnimation != animationId) {
		Log.info (F("Setting animation %d\n"), newAnimation);

		animationId = newAnimation;
		EEPROM.write (EEP_ANIMATION, animationId);
	}
}

void setBrightness (const int8_t diff) {
	int newBrightness = brightness + diff;
	if (newBrightness >= MIN_BRIGHTNESS && newBrightness <= MAX_BRIGHTNESS) {
		brightness = static_cast<byte> (newBrightness);
		EEPROM.write (EEP_BRIGHTNESS, brightness);
		ledController.setBrightness (brightness);
		Log.info (F("Brightness set to %d\n"), static_cast<int> (brightness));
	}
}

void reset () {
	Log.info (F("Resetting\n"));

	resetOutput.low ();
	delay (RESET_LENGTH_MS);
	resetOutput.high ();
}

void setMachineConfiguration (const byte newConfiguration) {
	if (newConfiguration != configuration && newConfiguration < MACHINE_SETTINGS_NO) {
		Log.info (F("Setting configuration %d\n"), static_cast<int> (newConfiguration));
		const auto& oldSettings = *static_cast<const MachineSettings*> (pgm_read_ptr (&machineSettings[configuration]));
		const auto& newSettings = *static_cast<const MachineSettings*> (pgm_read_ptr (&machineSettings[newConfiguration]));
		unsigned long start = millis ();

		if (newSettings.forceReset || newSettings.romSlot != oldSettings.romSlot) {
			Log.info (F("Reset start\n"));
			resetOutput.low ();
		}

		clockGenerator.setClock (0, newSettings.clock0);
		clockGenerator.setClock (1, newSettings.clock1);
		clockGenerator.setClock (2, newSettings.clock2);

		fastDigitalWrite (PIN_ROMSWITCH, newSettings.romSlot == 0 ? LOW : HIGH);

		SoftPWMSet (PIN_LED_R, newSettings.color.r);
		SoftPWMSet (PIN_LED_G, newSettings.color.g);
		SoftPWMSet (PIN_LED_B, newSettings.color.b);

		if (newSettings.forceReset || newSettings.romSlot != oldSettings.romSlot) {
			while (millis () - start < RESET_LENGTH_MS)
				;
			Log.info (F("Reset end\n"));
			resetOutput.high ();
		}

		configuration = newConfiguration;
		EEPROM.write (EEP_CONFIGURATION, configuration);
	}
}

/** \brief Updates matrix and generates key press/release events
 *
 * \param newBuf Keys currently being pressed
 */
void handleKeyboard (const KeyBuffer& newBuf) {
#if LOG_LEVEL_ENABLED (LOG_LEVEL_TRACE)
	if (newBuf.size > 0) {
		Log.debug (F("Handling new buffer:\n"));
		for (byte i = 0; i < newBuf.size; ++i) {
			Log.debug (F("- %X\n"), newBuf[i].key);
		}
		Log.debug (F("---\n"));
	}
#endif

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

	// R/G/B LED pins: configure as OUTPUTs and turn on
	fastPinConfig (PIN_LED_R, OUTPUT, HIGH);
	fastPinConfig (PIN_LED_G, OUTPUT, HIGH);
	fastPinConfig (PIN_LED_B, OUTPUT, HIGH);
	SoftPWMBegin ();
	// SoftPWMSetFadeTime (PIN_LED_R, 500, 500);
	// SoftPWMSetFadeTime (PIN_LED_G, 500, 500);
	// SoftPWMSetFadeTime (PIN_LED_B, 500, 500);

	// Other outputs
	fastPinConfig (PIN_ROMSWITCH, OUTPUT, LOW);

	// Apply startup machine configuration
	byte cfg = EEPROM.read (EEP_CONFIGURATION);
	if (cfg <= MACHINE_SETTINGS_NO) {
		configuration = cfg;
	} else {
		// Default clock
		configuration = 0;
	}
	clockGenerator.begin ();
	setMachineConfiguration (configuration);

	/* Wake up and configure the led controller ASAP, since it might show a random pattern at startup, and build the
	 * required coordinates array
	 */
	if (!ledController.begin () || !keyMap.begin ()) {
		Log.error (F("Unable to build the LED coordinates array, this indicates a mistake in the code\n"));

		// Hang with fast blinking
		while (true) {
			fastDigitalWrite (PIN_LED_R, HIGH);
			delay (222);
			fastDigitalWrite (PIN_LED_R, LOW);
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
	if (b <= static_cast<byte> (LightingMode::PRESSED_OFF)) {
		mode = static_cast<LightingMode> (b);
	} else {
		// Default mode
		mode = LightingMode::PRESSED_OFF;
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
	static unsigned long lastKeyboardScanTime = 0;
	static C16Key lastCombo = C16Key::NONE;

	// Check combos
	if (isPressed (C16Key::CMD) && isPressed (C16Key::CTRL)) {
		if (lastCombo == C16Key::NONE || !isPressed (lastCombo)) {		// Poor way to avoid key repetitions
			lastCombo = C16Key::NONE;
			for (const HotKey& hk: hotKeys) {
				if (isPressed (hk.key)) {
					lastCombo = hk.key;
					hk.action ();
					break;
				}
			}
		}
	}

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

		// lastKeyboardScanTime = millis ();
		lastKeyboardScanTime += KEYBOARD_SCAN_INTERVAL_MS;
	}
}
