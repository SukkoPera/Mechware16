/**
 * Copyright (c) 2024 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * \file MatrixKeyboardScanner.h
 * \author SukkoPera <software@sukkology.net>
 * \date 22 Feb 2020
 * \brief Generic Matrix-Keyboard Scanner
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MechBoard16
 */

#pragma once

#include "config.h"
#include "Matrix.h"
#include "KeyboardScanner.h"
#include "Log.h"

/******************************************************************************/

/** \brief Abstract output port for matrix scanning (Parent class)
 * 
 * Output ports provide the ability to set all their lines to inputs with
 * pull-ups and to set one line at a time low. Then, by reading the
 * corresponding #AbstractInputPort we can deduce the state of every key in the
 * matrix.
 */
//~ class AbstractOutputPort {
//~ public:
	//~ /** \brief Initialize the port
	 //~ * 
	 //~ * This shall set all lines to pulled-up inputs (possibly with a call to
	 //~ * #clearAllBits()), at least.
	 //~ */
	//~ virtual void begin () = 0;
	
	//~ /** \brief Bring a single line low
	 //~ * 
	 //~ * \param[in] n The line to bring low
	 //~ */
	//~ virtual void setBit (byte n) = 0;

	//~ //! \brief Set all lines to pulled-up inputs
	//~ virtual void clearAllBits () = 0;
//~ };

/** \brief Abstract output port for matrix scanning (Template class)
 * 
 * This is just syntactic sugar that will allow us to use template
 * specialization.
 */
template <byte NBITS>
class OutputPort	/*: public AbstractOutputPort*/ {	// Inheritance is not needed, we rely on template specialization
	// Do not implement anything, so compilation will fail if this is actually instantiated (it is not supposed to be!)
};

//! \brief 8-bit output port for matrix scanning
template <>
class OutputPort<8>	/*: public AbstractOutputPort */ {
public:
	void begin () {
		clearAllBits ();
	}
	
	void setBit (byte n) {
		DDRB = 1 << n;	// Port B is all inputs except bit n
		PORTB = ~DDRB;	// Disable pull-up and bring line low
	}

	void clearAllBits () {
		DDRB  = 0x00;   // Port B all inputs...
		PORTB = 0xFF;   // ... with pull-ups		
	}
};

/******************************************************************************/

template <byte NBITS, typename RTYPE>
class InputPort {
	// Same as above, not supposed to be instantiated but specializations shall implement the following
//~ public:
	//~ static const byte size = NBITS;
	
	//~ void begin () = 0;
	
	//~ RTYPE read () = 0;
};

/** \brief 8-bit input port for matrix scanning
 * 
 * The port is obtained by "merging" bits of different ports of the ATmega:
 * - Bits 0-1 are bits 0-1 of port of port F
 * - Bits 2-7 are bits 2-7 of port D
 *
 */
template <>
class InputPort<8, byte> {
public:
	void begin () {
		// All inputs with pull-ups
		PORTD = 0xFF;
		DDRD = 0x00;
	}
	
	byte read () {
		return PIND;
	}
};

/******************************************************************************/

/** \brief Generic key mapper
 * 
 * Translate a matrix scan into keypresses.
 */
template<byte NUMROWS, byte NUMCOLS, typename TYPECOLS>
class KeyMapper {
private:
	/** \brief Keymap to be used
	 * 
	 * This is a <em>pointer to a matrix</em>, beware the weird syntax!
	 */
	const Key (*keymap)[NUMCOLS];
	
public:
	typedef MatrixBase<NUMROWS, TYPECOLS> Matrix;

	/** \brief Initialize the KeyMapper
	 *
	 * \param[in] mtx An initial matrix read the mapper can use to select what
	 *                configuration to start up in
	 * \return True if successful, false otherwise
	 */
	virtual boolean begin (const Matrix& mtx) {
		(void) mtx;
		return true;
	}

	/** \brief Sets/changes the keymap
	 * 
	 * \param[in] _keymap The keymap to be used from now on
	 */
	void setKeyMap (const word _keymap[NUMROWS][NUMCOLS]) {
		keymap = _keymap;
	}
	
	/** \brief Map a matrix to keypresses
	 * 
	 * \param[in] mtx The matrix to be mapped
	 * \param[out] kbuf The #KeyBuffer where the detected keypresses will be
	 *                  stored
	 * \return The number of keypresses detected
	 */
	virtual byte map (const Matrix& mtx, KeyBuffer& kbuf) {
		byte ret = 0;

		if (keymap) {
			/* Process all rows for key-codes */
			for (byte row = 0; row < NUMROWS; ++row) {
				for (TYPECOLS col = 0, mask = 1; col < NUMCOLS; ++col, mask <<= 1) {
					if ((mtx[row] & mask) == 0) {
						/* Key pressed! Read keyboard map */
#ifdef ENABLE_MATRIX_DEBUG
						Log.debug (F("Detected key pressed at row %d, col %d\n"), (int) row, (int) col);
#endif
#ifdef KEYMAPS_IN_FLASH
						const Key key = pgm_read_word (&keymap[row][col]);
#else
						const Key key = keymap[row][col];
#endif
						if (key != 0) {
							if (!kbuf.full ()) {
								KeyEvent evt {
									.key = key,
									.row = row,
									.col = col
								};
								kbuf.append (evt);
							} else {
								Log.error (F("Key buffer is full\n"));
							}
						} else {
							Log.warn (F("Skipping unmapped key\n"));
						}
					}
				}
			}

			ret = kbuf.size;
		}

		return ret;
	}
};

/******************************************************************************/

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
template<byte NUMROWS, byte NUMCOLS, typename TYPECOLS, byte DEBOUNCE_LENGTH, typename MAPPER_T>
class MatrixKeyboardScanner: public KeyboardScanner {
private:
	byte debounce = DEBOUNCE_LENGTH;

public:
	typedef MatrixBase<NUMROWS, TYPECOLS> Matrix;

protected:
	OutputPort<NUMROWS> outPort;			// Rows
	InputPort<NUMCOLS, TYPECOLS> inPort;	// Columns

	Matrix matrix;

	MAPPER_T mapper;

	/** \brief Clear the keyboard matrix
	 * 
	 * All matrix points are marked as released.
	 */
	void clearMatrix () {
		for (byte row = 0; row < NUMROWS; ++row) {
			matrix[row] = 0xFF;
		}
	}

public:
	virtual boolean begin () override {
		clearMatrix ();
		outPort.begin ();
		inPort.begin ();

		// Do an initial read we can provide mapper.begin() with
		while (scanMatrix () != SCAN_COMPLETE)
			;

		return mapper.begin (matrix);
	}

	virtual boolean end () override {
		return true;
	}
	
	/* This function scans the entire keyboard, debounces the keys, and
	 * if a key change has been found, a new report is generated, and the
	 * function returns true to signal the transfer of the report
	 */
	ScanStatus scanMatrix () {
		ScanStatus scanStatus = SCAN_IN_PROGRESS;

		/* Scan all rows */
		for (byte row = 0; row < NUMROWS; ++row) {
			// Set a single row to ground
			outPort.setBit (row);
			
			// Wait for things to settle and then read column output
			delayMicroseconds (30);
			TYPECOLS data = inPort.read ();

			// If a change was detected, activate debounce counter
			if (matrix[row] != data) {
				debounce = DEBOUNCE_LENGTH; 
			}

			// Store the result
			matrix[row] = data; 
		}
		outPort.clearAllBits ();

		// Count down, but avoid underflow
		if (debounce > 1) {
			debounce--;
		} else {
			// Readings are stable
			scanStatus = SCAN_COMPLETE;
		}

		return scanStatus;
	}

	virtual void loop () override {
		/* The debouncing algorithm needs the matrix to be scanned as often as
		 * possible
		 */
		scanMatrix ();
	}
	
	virtual ScanStatus scan (KeyBuffer& buf) override {
		ScanStatus scanStatus = scanMatrix ();
		if (scanStatus == SCAN_COMPLETE) {
			mapper.map (matrix, buf);
		}

		return scanStatus;
	}
};
