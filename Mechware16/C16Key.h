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

enum class C16Key: byte {
	// Numbers
	_0,
	_1,
	_2,
	_3,
	_4,
	_5,
	_6,
	_7,
	_8,
	_9,
	
	// Letters
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	
	// Function
	F1,
	F2,
	F3,
	HELP,
	
	// Cursor
	UP,
	DOWN,
	LEFT,
	RIGHT,
	
	// Others
	ASTERISK,
	AT,
	CLEAR,
	CMD,
	COLON,
	COMMA,
	CTRL,
	DEL,
	RETURN,
	EQUAL,
	ESC,
	MINUS,
	PERIOD,
	PLUS,
	POUND,
	RUNSTOP,
	SEMICOLON,
	SHIFT,
	SLASH,
	SPACE,

	// Fake key placeholder
	NONE = 0xFF
};

constexpr byte N_PHYSICAL_KEYS = static_cast<byte> (C16Key::SPACE) + 1;

