/**
 * Copyright (c) 2024-2025 SukkoPera <software@sukkology.net>
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
#include "AnimationChasing.h"
#include "C16Key.h"
#include "MatrixCoordinates.h"

//! Order in which keys appear on the keyboard (+1 for 2 Shifts)
constexpr C16Key splash_order[N_PHYSICAL_KEYS + 1] PROGMEM = {
	C16Key::ESC, C16Key::_1, C16Key::_2, C16Key::_3, C16Key::_4, C16Key::_5, C16Key::_6, C16Key::_7, C16Key::_8, C16Key::_9, C16Key::_0, C16Key::LEFT, C16Key::RIGHT, C16Key::UP, C16Key::DOWN, C16Key::DEL,
	C16Key::CTRL, C16Key::Q, C16Key::W, C16Key::E, C16Key::R, C16Key::T, C16Key::Y, C16Key::U, C16Key::I, C16Key::O, C16Key::P, C16Key::AT, C16Key::PLUS, C16Key::MINUS, C16Key::CLEAR,
	C16Key::RUNSTOP, /* Shift Lock */ C16Key::A, C16Key::S, C16Key::D, C16Key::F, C16Key::G, C16Key::H, C16Key::J, C16Key::K, C16Key::L, C16Key::COLON, C16Key::SEMICOLON, C16Key::ASTERISK, C16Key::RETURN,
	C16Key::CMD, C16Key::SHIFT, C16Key::Z, C16Key::X, C16Key::C, C16Key::V, C16Key::B, C16Key::N, C16Key::M, C16Key::COMMA, C16Key::PERIOD, C16Key::SLASH, C16Key::SHIFT, C16Key::POUND, C16Key::EQUAL,
	C16Key::SPACE,
	C16Key::HELP, C16Key::F3, C16Key::F2, C16Key::F1    // Reverse order just to be cool ;)
};

void AnimationChasing::begin (LedControl& lc_) {
	lc = &lc_;
	i = 0;
}

boolean AnimationChasing::step () {
	for (byte j = 0; j < 3; ++j) {
		const byte k = pgm_read_byte (&(splash_order[i + j]));
		const MatrixCoordinates& pos = ledCoordinates[k];
		lc -> setLed (0, pos.row, pos.col, true);
	}
	
	delay (40);

	const byte k = pgm_read_byte (&(splash_order[i]));
	const MatrixCoordinates& pos = ledCoordinates[k];
	lc -> setLed (0, pos.row, pos.col, false);

	return ++i < N_PHYSICAL_KEYS + 1;
}
