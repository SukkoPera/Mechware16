/**
 * Copyright (c) 2024 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * \file AnimationScrollingColumn.cpp
 * \author SukkoPera <software@sukkology.net>
 * \date 11 Feb 2024
 * \brief Startup animation with a led chasing another (?)
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MechBoard16
 */

#include <avr/pgmspace.h>
#include "AnimationScrollingColumn.h"
#include "C16Key.h"
#include "MatrixCoordinates.h"

constexpr byte splash0nCols = 18;

//! Number of physical rows of keys
constexpr byte N_PHYSICAL_ROWS = 5;

// Lists of keys per each row, somehow lined up in "diagonal columns"
const C16Key splash0row4[splash0nCols] PROGMEM = {C16Key::NONE,    C16Key::ESC,   C16Key::_1,   C16Key::_2,    C16Key::_3,   C16Key::_4,   C16Key::_5,    C16Key::_6,   C16Key::_7,   C16Key::_8,    C16Key::_9,     C16Key::_0,    C16Key::LEFT,      C16Key::RIGHT,    C16Key::UP,     C16Key::DOWN,   C16Key::DEL,   C16Key::F1};
const C16Key splash0row3[splash0nCols] PROGMEM = {C16Key::NONE,    C16Key::CTRL,  C16Key::Q,    C16Key::W,     C16Key::E,    C16Key::R,    C16Key::T,     C16Key::Y,    C16Key::U,    C16Key::I,     C16Key::O,      C16Key::P,     C16Key::AT,        C16Key::PLUS,     C16Key::MINUS,  C16Key::NONE,   C16Key::CLEAR, C16Key::F2};
const C16Key splash0row2[splash0nCols] PROGMEM = {C16Key::RUNSTOP, C16Key::NONE,  C16Key::A,    C16Key::S,     C16Key::D,    C16Key::F,    C16Key::G,     C16Key::H,    C16Key::J,    C16Key::K,     C16Key::L,      C16Key::COLON, C16Key::SEMICOLON, C16Key::ASTERISK, C16Key::NONE,   C16Key::RETURN, C16Key::NONE,  C16Key::F3};
const C16Key splash0row1[splash0nCols] PROGMEM = {C16Key::CMD,     C16Key::SHIFT, C16Key::Z,    C16Key::X,     C16Key::C,    C16Key::V,    C16Key::B,     C16Key::N,    C16Key::M,    C16Key::COMMA, C16Key::PERIOD, C16Key::SLASH, C16Key::NONE,      C16Key::SHIFT,    C16Key::POUND,  C16Key::EQUAL,  C16Key::NONE,  C16Key::HELP};
const C16Key splash0row0[splash0nCols] PROGMEM = {C16Key::NONE,    C16Key::NONE,  C16Key::NONE, C16Key::SPACE, C16Key::NONE, C16Key::NONE, C16Key::SPACE, C16Key::NONE, C16Key::NONE, C16Key::SPACE, C16Key::NONE,   C16Key::NONE,  C16Key::NONE,      C16Key::NONE,     C16Key::NONE,   C16Key::NONE,   C16Key::NONE,  C16Key::NONE};

constexpr C16Key const * splash0rows[N_PHYSICAL_ROWS] PROGMEM = {splash0row4, splash0row3, splash0row2, splash0row1, splash0row0};


void AnimationScrollingColumn::begin (LedControl& lc_) {
	lc = &lc_;
	i = 0;
	j = 0;
}

boolean AnimationScrollingColumn::step () {
	// Turn on a column...
	for (byte j = 0; j < N_PHYSICAL_ROWS; ++j) {
		const byte* krow = pgm_read_byte (&splash0rows[j]);
		const byte k = pgm_read_byte (&(krow[i]));
		if (static_cast<C16Key> (k) != C16Key::NONE) {
			const MatrixCoordinates& pos = ledCoordinates[k];
			lc -> setLed (0, pos.row, pos.col, true);
		}
	}

	// ... wait a bit...
	delay (60);

	// ... and turn it off
	for (byte j = 0; j < N_PHYSICAL_ROWS; ++j) {
		const byte* krow = pgm_read_byte (&splash0rows[j]);
		const byte k = pgm_read_byte (&(krow[i]));
		if (static_cast<C16Key> (k) != C16Key::NONE) {
			const MatrixCoordinates& pos = ledCoordinates[k];
			lc -> setLed (0, pos.row, pos.col, false);
		}
	}

	return ++i < splash0nCols;
}
