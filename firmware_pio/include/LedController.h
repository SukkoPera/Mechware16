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

#include "C16Key.h"
#include "MatrixCoordinates.h"

#include <LedControl.h>

class LedController {
public:
    LedController () = default;
    virtual ~LedController () = default;

    virtual boolean begin() = 0;

    virtual void setLedForKey(C16Key key, bool on) = 0;

    // This is provided in case there is a faster way to turn all leds on/off at the same time
    // TODO: Provide dumb implementation
    virtual void setAllLeds(bool on) = 0;

    // 0-15
    virtual void setBrightness(byte brightness) = 0;
};

class SimpleLedController: public LedController {
public:
    SimpleLedController();
    ~SimpleLedController() = default;

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
