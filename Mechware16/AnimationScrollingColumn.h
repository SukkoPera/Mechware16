/**
 * Copyright (c) 2024-2025 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * \file AnimationScrollingColumn.h
 * \author SukkoPera <software@sukkology.net>
 * \date 11 Feb 2024
 * \brief Startup animation with a led chasing another (?)
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MechBoard16
 */

#pragma once

#include "Animation.h"

class AnimationScrollingColumn: public Animation {
public:
	virtual void begin (LedControl& lc_) override;
	virtual boolean step () override;

private:
	LedControl *lc;

	byte i;
	byte j;
};
