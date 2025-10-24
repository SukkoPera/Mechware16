/**
 * Copyright (c) 2024-2025 SukkoPera <software@sukkology.net>
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

#include "KeyMapperC16.h"
#include "MatrixKeyboardScanner.h"

//! \brief C16/Plus4 keyboard scanner
class KbdScannerC16: public MatrixKeyboardScanner<C16_MATRIX_ROWS, C16_MATRIX_COLS, byte, DEBOUNCE_FACTOR_C16, KeyMapperC16> {
public:

	virtual void updateLeds (const boolean capsLock, const boolean numLock, const boolean scrollLock) override {
		// Update caps lock led status
		//~ fastDigitalWrite (LED_PIN, capsLock || numLock || scrollLock);
	}
};
