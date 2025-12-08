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

enum class Clock: byte {
	PAL,
	NTSC
};

class ClockGenerator {
public:
	void begin (Clock clock);

	void setClock (Clock clock);

private:
	static constexpr uint32_t QUARTZ_FREQUENCY = 25000000UL;

	static constexpr uint64_t CLOCK_FREQUENCIES[] = {
		17734475ULL,	// PAL
		14318181ULL		// NTSC
	};
	
	Si5351 si5351;
};
