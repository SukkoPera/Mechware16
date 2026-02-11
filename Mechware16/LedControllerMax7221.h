/**
 * ___  ___          _    ______                     _  __    ____
 * |  \/  |         | |   | ___ \                   | |/  |  / ___|
 * | .  . | ___  ___| |__ | |_/ / ___   __ _ _ __ __| |`| | / /___
 * | |\/| |/ _ \/ __| '_ \| ___ \/ _ \ / _` | '__/ _` | | | | ___ \
 * | |  | |  __/ (__| | | | |_/ / (_) | (_| | | | (_| |_| |_| \_/ |
 * \_|  |_/\___|\___|_| |_\____/ \___/ \__,_|_|  \__,_|\___/\_____/
 *
 * Copyright (c) 2024-2025 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#ifdef ENABLE_LEDCONTROLLER_MAX7221

#include "LedController.h"
#include "MatrixCoordinates.h"

#include <LedControl.h>

class LedControllerMax7221: public LedController {
public:
    LedControllerMax7221();
    ~LedControllerMax7221() = default;

    boolean begin() override;

    void setLedForKey(C16Key key, bool on) override;

    void setAllLeds(bool on) override;

    void setBrightness(byte brightness) override;

private:
    /* Maps a key to the (row, col) tuple that controls its led.
     * Built by buildLedCoordinates().
     */
    MatrixCoordinates ledCoordinates[N_PHYSICAL_KEYS];

    LedControl lc;

    boolean buildLedCoordinates ();
};

#endif
