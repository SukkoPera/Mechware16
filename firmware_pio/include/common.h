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

#pragma once

#include <Arduino.h>

constexpr byte PIN_LED_R = A5;
constexpr byte PIN_LED_G = A4;
constexpr byte PIN_LED_B = A3;
constexpr byte PIN_MAX7221_SEL = A2;
constexpr byte PIN_MAX7221_DATA = A1;
constexpr byte PIN_MAX7221_CLK = A0;

// TODO: Make another KeyboardScanner with this stuff
/** \brief Number of rows in the C16/Plus4 keyboard matrix
 */
constexpr byte MATRIX_ROWS = 8;

/** \brief Number of columns in the C16/Plus4 keyboard matrix
 */
constexpr byte MATRIX_COLS = 8;

// Min/max brightness values
constexpr byte MIN_BRIGHTNESS = 0;
constexpr byte MAX_BRIGHTNESS = 15;
