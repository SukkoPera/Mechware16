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

#include "Log.h"
extern Logging Log;

constexpr uint64_t ClockGenerator::CLOCK_FREQUENCIES[];

void ClockGenerator::begin (Clock clock) {
  // Initialize the Si5351
  if (si5351.init(SI5351_CRYSTAL_LOAD_8PF, QUARTZ_FREQUENCY, 0)) {
	  si5351.set_clock_pwr(SI5351_CLK0, 1);
	  si5351.set_clock_pwr(SI5351_CLK1, 0);
	  si5351.set_clock_pwr(SI5351_CLK2, 0);

	  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);

	  si5351.set_freq(CLOCK_FREQUENCIES[static_cast<unsigned int> (clock)] * SI5351_FREQ_MULT, SI5351_CLK0);

	  si5351.output_enable(SI5351_CLK0, 1);
	  si5351.output_enable(SI5351_CLK1, 0);
	  si5351.output_enable(SI5351_CLK2, 0);

	  Log.error(F("Clock generator initialized\n"));
  } else {
	  Log.error(F("Cannot init clock generator\n"));
  }
}

void ClockGenerator::setClock (Clock clock) {
	si5351.set_freq(CLOCK_FREQUENCIES[static_cast<unsigned int> (clock)] * SI5351_FREQ_MULT, SI5351_CLK0);
}
