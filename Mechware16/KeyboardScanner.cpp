/**
 * Copyright (c) 2024-2025 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
 
/**
 * \file KeyboardScanner.cpp
 * \author SukkoPera <software@sukkology.net>
 * \date 22 Feb 2020
 * \brief Abstract Keyboard Scanner (Parent Class)
 * \ingroup KeyboardScanners
 */

#include <Arduino.h>
#include "KeyboardScanner.h"

boolean eventKeyCompare (const KeyEvent& evt, const Key& k) {
	return evt.key == k;
}
