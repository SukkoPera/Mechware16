/**
 * Copyright (c) 2024-2025 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * \file KbdScannerPassive16.cpp
 * \author SukkoPera <software@sukkology.net>
 * \date 29 Jan 2024
 * \brief Commodore 16/Plus4 Passive Keyboard Scanner
 * \ingroup KeyboardScanners
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MechBoard16
 */
#include "config.h"
#include "KbdScannerPassive16.h"
#include "CircularBuffer.h"
#include "UsbKeyboard.h"
#include "Log.h"

struct KeyMatrixSample {
	byte rows;			// If this is made a uint8_t we save 6 bytes, how come?!?
	byte cols;
};

/* volatile */ static CircularBuffer<KeyMatrixSample, byte, 32> matrixSamples;


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


unsigned int KbdScannerPassive16::countSetBits (int n) { 
	unsigned int count = 0; 

	while (n) { 
		n &= n - 1;
		++count; 
	} 

	return count; 
}

void KbdScannerPassive16::clearMatrix () {
	for (byte row = 0; row < MATRIX_ROWS; ++row) {
		matrix[row] = 0xFF;
	}
}

boolean KbdScannerPassive16::begin () {
	clearMatrix ();
	inPort.begin ();

	/* Keyboard polling: TED drives the rows, which we have on PORT B, while the keyboard "outputs" the columns, which
	 * we have on PORT D. They are all INPUTs by default se all we have to do is to enable the pin-change interrupts on
	 * all pins of PORT B.
	 */
	noInterrupts ();
	PCMSK0 = (1 << PCINT7) | (1 << PCINT6) | (1 << PCINT5) | (1 << PCINT4) |
			 (1 << PCINT3) | (1 << PCINT2) | (1 << PCINT1) | (1 << PCINT0);
	PCICR |= (1 << PCIE0);
	// TODO: Clear pending interrupt flag?
	interrupts ();

	return mapper.begin (matrix);
}

boolean KbdScannerPassive16::end () {
	PCICR &= ~(1 << PCIE0);		// Disable pin-change interrupts
	return true;
}

// TODO: Debouncing?
void KbdScannerPassive16::loop () {
	/* The debouncing algorithm needs the matrix to be scanned as often as
	 * possible
	 */
	while (matrixSamples.available ()) {
		KeyMatrixSample sample = matrixSamples.get ();

		/* When scanning the keyboard, the C16/+4 KERNAL first does a quick test to check if any key is pressed at all:
		 * it brings all the rows down and checks whether all cols are up or not. If at least one column is down, it
		 * goes on to check every individual row, as at least one key must be pressed and there's no other way to find
		 * out exactly which one, otherwise it takes no further action and just terminates the scan there.
		 */
		if (sample.rows == 0x00 && sample.cols == 0xFF) {
			// All keys released
			clearMatrix ();
		} else if (countSetBits (sample.rows) == 7) {
			// Exactly one row is cleared, find out which one and update all its columns
			for (byte row = 0; row < MATRIX_ROWS; ++row) {
				if ((sample.rows & (1 << row)) == 0) {
					matrix[row] = sample.cols;
					break;		// There is necessarily only one row at 0
				}
			}
		}
	}
}

KeyboardScanner::ScanStatus KbdScannerPassive16::scan (KeyBuffer& buf) {
	ScanStatus scanStatus = matrixSamples.available () ? SCAN_IN_PROGRESS : SCAN_COMPLETE;
	if (scanStatus == SCAN_COMPLETE) {
		mapper.map (matrix, buf);
	}

	return scanStatus;
}
