/**
 * Copyright (c) 2025 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * \file ClockGenerator.h
 * \author SukkoPera <software@sukkology.net>
 * \date 10 Jun 2025
 * \brief Clock Generation
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MechBoard16
 */

#pragma once

#include "si5351.h"
#include "types.h"

class ClockGenerator {
public:
	void begin ();

	void setClock (byte clockId, Clock clock);

private:
	static constexpr uint32_t QUARTZ_FREQUENCY = 25000000UL;

	// Keep these in the same order as enum Clock from types.h
	static constexpr uint64_t CLOCK_FREQUENCIES[] = {
		0,				// Disabled
		17734475ULL,	// System clock, PAL
		14318181ULL,	// System clock, NTSC
		1843200,		// ACIA Normal
		3686400,		// ACIA Double
		7372800,		// ACIA Quad
		3000000,		// ACIA Midi Hack
	};
	
	Si5351 si5351;

	void setFrequency (si5351_clock clockId, uint64_t freq);
};
