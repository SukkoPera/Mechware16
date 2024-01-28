/**
 * Copyright (c) 2024 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * \file KbdScannerC16.h
 * \author SukkoPera <software@sukkology.net>
 * \date 22 Feb 2020
 * \brief Commodore 16/Plus4 Keyboard Scanner
 * \ingroup KeyboardScanners
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MechBoard16
 */

#pragma once

#include "config.h"
#include "MatrixKeyboardScanner.h"
#include "UsbKeyboard.h"
#include "Log.h"

/** \brief Number of rows in the C16/Plus4 keyboard matrix
 * 
 * \sa C16_MATRIX_COLS
 */
#define C16_MATRIX_ROWS 8

/** \brief Number of columns in the C16/Plus4 keyboard matrix
 * 
 * \sa C16_MATRIX_ROWS
 */
#define C16_MATRIX_COLS 8

/** \brief C16/Plus4/(Probably C116) keyboard mapper
 * 
 * The C16 keyboard has 66 keys, but the two Shift keys and Caps Lock are all
 * connected to the same line. This means all the keys are arranged in a matrix
 * of 8 rows and 8 columns.
 * 
 * The Plus/4 has an additional Control key on the right side of the keyboard,
 * but this is connected to the same line as the Control key on the left side,
 * thus the matrix is essentially the same.
 * 
 * The Plus/4 also has an additional Power Led that is wired in the keyboard
 * connector, while it is separate on the C16. No idea on the C116.
 * 
 * This mapper works the same as the one for the C64 (#KeyMapperC64), so please
 * refer to that for any information.
 */
class KeyMapperC16: public KeyMapper<C16_MATRIX_ROWS, C16_MATRIX_COLS, byte> {
private:
	enum KbdMode {
		KBD_POSITIONAL,
		KBD_SYMBOLIC
	};

	KbdMode kmode;
	
	// C16, Positional Mapping with our own mapping settings
	static const Key keymapPositional[C16_MATRIX_ROWS][C16_MATRIX_COLS] PROGMEM;
	
	static const Key keymapSymbolic[C16_MATRIX_ROWS][C16_MATRIX_COLS] PROGMEM;

	static const Key keymapSymbolicShifted[C16_MATRIX_ROWS][C16_MATRIX_COLS] PROGMEM;
	
	KbdMode getStartupMode (const Matrix& mtx) const {
		KbdMode md = KBD_SYMBOLIC;
		if ((mtx[7] & (1 << 5)) == 0) {
			// C= key pressed
			md = KBD_POSITIONAL;
		}

		return md;
	}
	
public:
	virtual boolean begin (const Matrix& mtx) override {
		switch ((kmode = getStartupMode (mtx))) {
			case KBD_POSITIONAL:
				Log.info (F("Starting up in POSITIONAL mode\n"));
				setKeyMap (keymapPositional);
				break;
			case KBD_SYMBOLIC:
			default:
				Log.info (F("Starting up in SYMBOLIC mode\n"));
				// Nothing to do
				break;
		}
		return KeyMapper<C16_MATRIX_ROWS, C16_MATRIX_COLS, byte>::begin (mtx);
	}

	virtual byte map (const Matrix& mtx, KeyBuffer& kbuf) override {
		byte ret = 0;
		
		if (kmode == KBD_POSITIONAL) {
			ret = KeyMapper<C16_MATRIX_ROWS, C16_MATRIX_COLS, byte>::map (mtx, kbuf);
		} else {
			if ((mtx[7] & (1 << 1)) == 0) {
				// Shift is pressed
				setKeyMap (keymapSymbolicShifted);
			} else {
				setKeyMap (keymapSymbolic);
			}

			ret = KeyMapper<C16_MATRIX_ROWS, C16_MATRIX_COLS, byte>::map (mtx, kbuf);

			// See if we need to remove the SHIFT key from the buffer
			if (kbuf.size > 1 && kbuf.find (KEY_LEFT_SHIFT) >= 0) {
				boolean remove = false;
				for (byte i = 0; i < kbuf.size && !remove; ++i) {
					switch (kbuf[i]) {
					case KEY_LEFT_SHIFT:
					case KEY_LEFT_CTRL:
					case KEY_LEFT_ALT:
					case KEY_UP:
					case KEY_DOWN:
					case KEY_LEFT:
					case KEY_RIGHT:
					case KEY_HOME:
					case KEY_TAB:
					case KEY_ESC:
						// These keys can be pressed with SHIFT freely
						break;
					case KEY_F1 ... KEY_F8:
						/* The function keys change their meaning with shift, so
						 * we'd better remove it
						 */
						remove = true;
						break;
					default:
						/* Some keys require shift to be (de)synthesized, so
						 * let's pretend it's not pressed
						 */
						if (!UsbKeyboard::keyNeedsShift (kbuf[i])) {
							remove = true;
						}
						break;
					}
				}

				if (remove) {
					kbuf.remove (KEY_LEFT_SHIFT);
				}
			}
		}

		return ret;
	}
};

//! \brief C16/Plus4 keyboard scanner
class KbdScannerC16: public MatrixKeyboardScanner<C16_MATRIX_ROWS, C16_MATRIX_COLS, byte, DEBOUNCE_FACTOR_C16, KeyMapperC16> {
public:		
};
