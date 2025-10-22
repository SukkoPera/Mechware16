/**
 * Copyright (c) 2024 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
 
/**
 * \file UsbKeyboard.h
 * \author SukkoPera <software@sukkology.net>
 * \date 22 Feb 2020
 * \brief Abstract USB Keyboard Interface (for output)
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MechBoard16
 */

#pragma once

#include <HID-Project.h>

const uint16_t MASK_ASCIIKEY = (uint16_t) (1U << 15);
#define A(c) ((c) | (MASK_ASCIIKEY))
#define KEYPRESS_IS_ASCII(k) ((k) & MASK_ASCIIKEY)
#define ASCII_EXTRACT(c) static_cast<uint8_t> ((c) & 0xFF)

// Use same values as HID-Project's KeyboardLeds
enum UsbKeyboardLeds {
	USBLED_NUM_LOCK		= (1 << 0),
	USBLED_CAPS_LOCK	= (1 << 1),
	USBLED_SCROLL_LOCK	= (1 << 2)
};

class UsbKeyboard {
public:
	boolean begin () {
		BootKeyboard.begin ();
  
		return true;
	}

	byte getLeds () {
		return BootKeyboard.getLeds ();
	}

	boolean press (uint16_t n) {
		boolean ret;

		if (KEYPRESS_IS_ASCII (n)) {
			uint8_t k = ASCII_EXTRACT (n);
			ret = BootKeyboard.add (k);
		} else {
			KeyboardKeycode k = static_cast<KeyboardKeycode> (n);
			ret = BootKeyboard.add (k);
		}

		return ret;
	}

	boolean release (uint16_t n) {
		boolean ret;

		if (KEYPRESS_IS_ASCII (n)) {
			uint8_t k = ASCII_EXTRACT (n);
			ret = BootKeyboard.remove (k);
		} else {
			KeyboardKeycode k = static_cast<KeyboardKeycode> (n);
			ret = BootKeyboard.remove (k);
		}

		return ret;
	}

	boolean commit () {
		return BootKeyboard.send ();
	}

	static boolean keyNeedsShift (uint16_t k) {
		boolean needed = false;

		if (k < sizeof (_asciimap)) {
			// Read key from ascii lookup table
			uint16_t key = pgm_read_word (_asciimap + k);
			
			if (key & (MOD_LEFT_SHIFT | MOD_RIGHT_SHIFT)) {
				needed = true;
			}
		}

		return needed;
	}
};
