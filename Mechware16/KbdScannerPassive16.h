/**
 * Copyright (c) 2024-2025 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * \file KbdScannerPassive16.h
 * \author SukkoPera <software@sukkology.net>
 * \date 22 Feb 2020
 * \brief Commodore 16/Plus4 Passive Keyboard Scanner
 * \ingroup KeyboardScanners
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MechBoard16
 */

#pragma once

//~ #include "config.h"
#include "KeyMapperC16.h"
#include "MatrixKeyboardScanner.h"

/** \brief Generic matrix keyboard scanner
 * 
 * Theory of operation:
 * - Initially, all keyboard scan keys are set as inputs, but with pullups. This
 *   causes them to be "weak" +5 outputs.
 * - To scan, we set one line as a low output, causing it to "win" over the weak
 *   outputs.
 * - The resulting matrix is finally fed to a #KeyMapper, which translates it
 *   into the actual keypresses.
 */
class KbdScannerPassive16: public KeyboardScanner {
public:
	typedef MatrixBase<C16_MATRIX_ROWS, byte> Matrix;

private:
	InputPort<C16_MATRIX_COLS, byte> inPort;	// Columns

	Matrix matrix;

	KeyMapperC16 mapper;

	/** \brief Get number of set bits in the binary representation of a number
	 * 
	 * All hail to Brian Kernighan.
	 * 
	 * \param[in] n The number
	 * \return The number of bits set
	 */
	static unsigned int countSetBits (int n);

	/** \brief Clear the keyboard matrix
	 * 
	 * All matrix points are marked as released.
	 */
	void clearMatrix () ;
	
public:
	virtual boolean begin () override;
	virtual boolean end () override;
	
	virtual void loop () override;
	
	virtual KeyboardScanner::ScanStatus scan (KeyBuffer& buf) override;
};

