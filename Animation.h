/**
 * Copyright (c) 2024 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * \file Animation.h
 * \author SukkoPera <software@sukkology.net>
 * \date 11 Feb 2024
 * \brief Generic startup animation
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MechBoard16
 */

#pragma once

#include <Arduino.h>
#include "LedControl.h"

class Animation {
public:
	virtual void begin (LedControl& lc_) = 0;
	virtual boolean step () = 0;
};
