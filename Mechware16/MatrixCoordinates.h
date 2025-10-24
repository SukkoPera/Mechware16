/**
 * Copyright (c) 2024-2025 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * \file C16Key.h
 * \author SukkoPera <software@sukkology.net>
 * \date 11 Feb 2024
 * \brief Commodore 16 key definitions
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MechBoard16
 */

#pragma once

#include <Arduino.h>

struct MatrixCoordinates {
	byte row;
	byte col;
};

// TODO: Probably not the best place for these extern declarations

/* Maps a key to the (row, col) tuple that controls its led.
 * Built by buildLedCoordinates().
 */
extern MatrixCoordinates ledCoordinates[N_PHYSICAL_KEYS];

/* Maps a key to the (row, col) tuple describing its position in the keyboard matrix, useful for quick lookups.
 * Built by buildKeyCoordinates().
 */
extern MatrixCoordinates keyCoordinates[N_PHYSICAL_KEYS];
