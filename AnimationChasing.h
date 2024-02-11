/**
 * Copyright (c) 2024 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * \file AnimationChasing.h
 * \author SukkoPera <software@sukkology.net>
 * \date 11 Feb 2024
 * \brief Startup animation with some leds chasing each other
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MechBoard16
 */

#pragma once

#include "Animation.h"

class AnimationChasing: public Animation {
public:
	virtual void begin (LedControl& lc_) override;
	virtual boolean step () override;

private:
	LedControl *lc;

	byte i;
};
