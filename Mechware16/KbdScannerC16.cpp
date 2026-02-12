/**
 * Copyright (c) 2024-2025 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
 
/**
 * \file KbdScannerC16.cpp
 * \author SukkoPera <software@sukkology.net>
 * \date 22 Feb 2020
 * \brief Commodore 16/Plus4 Keyboard Scanner
 * \ingroup KeyboardScanners
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MechBoard16
 */

#include <Arduino.h>
#include "config.h"
#include "KbdScannerC16.h"
#include "UsbKeyboard.h"

#ifndef ENABLE_EURO_KEY
constexpr Key POUND_SIGN = A(163);
#else
constexpr Key POUND_SIGN = A(164);
#endif

constexpr Key KeyMapperC16::keymapPositional[MATRIX_ROWS][MATRIX_COLS] = {
	{KEY_BACKSPACE,		KEY_ENTER,			KEY_EQUAL,		KEY_F8,		KEY_F1,		KEY_F2,			KEY_F3,				KEY_LEFT_BRACE},
	{KEY_3,				KEY_W,				KEY_A,			KEY_4,		KEY_Z,		KEY_S,			KEY_E,				KEY_LEFT_SHIFT},
	{KEY_5,				KEY_R,				KEY_D,			KEY_6,		KEY_C,		KEY_F,			KEY_T,				KEY_X},
	{KEY_7,				KEY_Y,				KEY_G,			KEY_8,		KEY_B,		KEY_H,			KEY_U,				KEY_V},
	{KEY_9,				KEY_I,				KEY_J,			KEY_0,		KEY_M,		KEY_K,			KEY_O,				KEY_N},
	{KEY_DOWN,			KEY_P,				KEY_L,			KEY_UP,		KEY_PERIOD,	KEY_SEMICOLON,	KEY_MINUS,			KEY_COMMA},
	{KEY_LEFT,			KEY_BACKSLASH,		KEY_QUOTE,		KEY_RIGHT,	KEY_TILDE,	KEY_INSERT,		KEY_RIGHT_BRACE,	KEY_SLASH},
	{KEY_1,				KEY_HOME,			KEY_TAB,		KEY_2,		KEY_SPACE,	KEY_LEFT_CTRL,	KEY_Q,				KEY_ESC}
};

#ifdef ENABLE_USB
const Key KeyMapperC16::keymapSymbolic[MATRIX_ROWS][MATRIX_COLS] = {
	{KEY_BACKSPACE,		KEY_ENTER,			POUND_SIGN,		KEY_F8,		KEY_F1,		KEY_F2,			KEY_F3,				A('@')},
	{A('3'),			A('w'),				A('a'),			A('4'),		A('z'),		A('s'),			A('e'),				KEY_LEFT_SHIFT},
	{A('5'),			A('r'),				A('d'),			A('6'),		A('c'),		A('f'),			A('t'),				A('x')},
	{A('7'),			A('y'),				A('g'),			A('8'),		A('b'),		A('h'),			A('u'),				A('v')},
	{A('9'),			A('i'),				A('j'),			A('0'),		A('m'),		A('k'),			A('o'),				A('n')},
	{KEY_DOWN,			A('p'),				A('l'),			KEY_UP,		A('.'),		A(':'),			A('-'),				A(',')},
	{KEY_LEFT,			A('*'),				A(';'),			KEY_RIGHT,	KEY_ESC,	A('='),			A('+'),				A('/')},
	{A('1'),			KEY_HOME,			KEY_LEFT_CTRL,	A('2'),		A(' '),		KEY_LEFT_ALT,	A('q'),				KEY_TAB}
};

const Key KeyMapperC16::keymapSymbolicShifted[MATRIX_ROWS][MATRIX_COLS] = {
	{KEY_INSERT,		KEY_ENTER,			POUND_SIGN,		KEY_F7,		KEY_F4,		KEY_F5,			KEY_F6,				A('@')},
	{A('#'),			A('W'),				A('A'),			A('$'),		A('Z'),		A('S'),			A('E'),				KEY_LEFT_SHIFT},
	{A('%'),			A('R'),				A('D'),			A('&'),		A('C'),		A('F'),			A('T'),				A('X')},
	{A('\''),			A('Y'),				A('G'),			A('('),		A('B'),		A('H'),			A('U'),				A('V')},
	{A(')'),			A('I'),				A('J'),			A('^'),		A('M'),		A('K'),			A('O'),				A('N')},
	{KEY_DOWN,			A('P'),				A('L'),			KEY_UP,		A('>'),		A('['),			A('-'),				A('<')},
	{KEY_LEFT,			A('*'),				A(']'),			KEY_RIGHT,	KEY_ESC,	A('='),			A('+'),				A('?')},
	{A('!'),			KEY_HOME,			KEY_LEFT_CTRL,	A('\"'),	A(' '),		KEY_LEFT_ALT,	A('Q'),				KEY_TAB}
};
#endif
