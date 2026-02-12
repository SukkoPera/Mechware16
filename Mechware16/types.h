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

#include <Arduino.h>
#include "C16Key.h"

enum class LightingMode: byte {
    ALWAYS_OFF,
    ALWAYS_ON,
    PRESSED_ON,
    PRESSED_OFF
};

enum class Clock: byte {
    DISABLED = 0,
    SYS_PAL,
    SYS_NTSC,
    ACIA_NORMAL,			// Normal ACIA clock resulting in max 19200 bps
    ACIA_DOUBLE,			// Double ACIA clock, 38400 bps theoretically possible
    ACIA_QUAD,				// 76800 bps ;)
    ACIA_MIDIHACK,			/* This allows reaching the 31250 bps required for midi configuring the acia for 19200 bps
                             * (divisor = 96 => 3000000/96 = 31250)
                             */
};

struct Color {
    byte r;
    byte g;
    byte b;
};

struct MachineSettings {
    Clock clock0;			// System Clock
    Clock clock1;			// ACIA Clock
    Clock clock2;			// Unused
    byte romSlot;
    Color color;			// LED Color
    boolean forceReset;		// Force reset upon entering this configuration, otherwise it's only performed when ROM slot changes
};

struct HotKey {
    C16Key key;
    void (*action) ();
};
