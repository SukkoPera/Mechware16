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

#include "ClockGenerator.h"

#include <time.h>

#include "Log.h"
extern Logging Log;

constexpr uint64_t ClockGenerator::CLOCK_FREQUENCIES[];

void ClockGenerator::begin () {
	// Initialize the Si5351
	if (si5351.init (SI5351_CRYSTAL_LOAD_8PF, QUARTZ_FREQUENCY, 0)) {
		si5351.drive_strength (SI5351_CLK0, SI5351_DRIVE_8MA);
		si5351.drive_strength (SI5351_CLK1, SI5351_DRIVE_8MA);
		si5351.drive_strength (SI5351_CLK2, SI5351_DRIVE_8MA);

		Log.info (F("Clock generator initialized\n"));
	} else {
		Log.error (F("Cannot init clock generator\n"));
	}
}

void ClockGenerator::setClock (byte clockId, Clock clock) {
	Log.debug (F("Setting clock %d to %d\n"), static_cast<int> (clockId), static_cast<int> (clock));
	switch (clockId) {
		case 0:
			setFrequency(SI5351_CLK0, CLOCK_FREQUENCIES[static_cast<unsigned int> (clock)]);
			break;
		case 1:
			setFrequency(SI5351_CLK1, CLOCK_FREQUENCIES[static_cast<unsigned int> (clock)]);
			break;
		case 2:
			setFrequency(SI5351_CLK2, CLOCK_FREQUENCIES[static_cast<unsigned int> (clock)]);
			break;
		default:
			// Fail silently
			break;
	}
}

void ClockGenerator::setFrequency (si5351_clock clockId, uint64_t freq) {
	if (freq == 0) {
		si5351.set_freq (freq, clockId);
		si5351.output_enable (clockId, 0);
		si5351.set_clock_pwr (clockId, 0);
	} else {
		si5351.set_clock_pwr (clockId, 1);
		si5351.set_freq (freq * SI5351_FREQ_MULT, clockId);
		si5351.output_enable (clockId, 1);
	}
}
