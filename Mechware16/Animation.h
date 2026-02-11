/**
 * Copyright (c) 2024-2025 SukkoPera <software@sukkology.net>
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
#include "LedController.h"

class Animation {
public:
	Animation () = default;
	virtual ~Animation () = default;

	virtual void begin (LedController& lc_) = 0;
	virtual boolean step () = 0;
};
