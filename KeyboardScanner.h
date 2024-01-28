/**
 * Copyright (c) 2024 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
 
/**
 * \file KeyboardScanner.h
 * \author SukkoPera <software@sukkology.net>
 * \date 22 Feb 2020
 * \brief Abstract Keyboard Scanner (Parent Class)
 * \ingroup KeyboardScanners
 *
 * This is the abstract class that must be derived by all the various keyboard
 * scanners.
 */
 
/**
 * \defgroup KeyboardScanners Keyboard Scanners
 * 
 * \brief Keyboard scanning classes
 * 
 * These classes are in charge of scanning the different keyboard and reporting
 * keys being pressed and released.
 */

#pragma once

#include "config.h"
#include <Arduino.h>
#include "SmallBuffer.h"

/** \brief Size of keyboard buffer
 * 
 * This is implicitly the maximum number of keys pressed at once.
 */
const byte KEYBUF_SIZE = 6;

//! \brief Type used to represent keypresses
typedef word Key;

//! \brief Type used to report keypresses
typedef SmallBuffer<Key, KEYBUF_SIZE> KeyBuffer;

/** \brief Abstract Keyboard Scanner (Parent Class)
 * 
 * This is the abstract class that must be derived by all the various keyboard
 * scanners.
 */
class KeyboardScanner {
protected:
public:
	//! Keyboard scan result
	enum ScanStatus {
		SCAN_ERROR,				//!< Scan round failed
		SCAN_IN_PROGRESS,		//!< Scan still in progress, please come back later
		SCAN_COMPLETE			//!< Scan completed
	};
	
	//! \brief Initialize scanner
	virtual boolean begin () = 0;
	
	//! \brief Cleanup scanner
	virtual boolean end () = 0;

	/** \brief Do internal stuff
	 *
	 * This function will be called as often as possible and can be used by the
	 * scanner to do its internal housekeeping. A do-nothing default
	 * implementation is provided.
	 */
	virtual void loop () {
	}

	/** \brief Scan keyboard
	 * 
	 * This function shall scan the keyboard and report the keys currently being
	 * pressed.
	 * 
	 * \param[out] buf A buffer that must be filled with all the keys currently
	 *                 being pressed
	 * \return The scan result
	 */
	virtual ScanStatus scan (KeyBuffer& buf) = 0;
};
