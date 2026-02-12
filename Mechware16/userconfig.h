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

#include "types.h"

const MachineSettings machinePal PROGMEM {
    Clock::SYS_PAL,
    Clock::ACIA_NORMAL,
    Clock::DISABLED,
    0,
    {255, 0, 0},
    false
};

const MachineSettings machinePalJiffyDos PROGMEM {
    Clock::SYS_PAL,
    Clock::ACIA_NORMAL,
    Clock::DISABLED,
    1,
    {0, 255, 0},
    false
};

const MachineSettings machinePalJiffyDosFastAcia PROGMEM {
    Clock::SYS_PAL,
    Clock::ACIA_DOUBLE,
    Clock::DISABLED,
    1,
    {0, 255, 127},
    false
};

const MachineSettings machinePalJiffyDosMidiHack PROGMEM {
    Clock::SYS_PAL,
    Clock::ACIA_MIDIHACK,
    Clock::DISABLED,
    1,
    {127, 255, 127},
    false
};

// const MachineSettings machineNtsc PROGMEM {
//     Clock::SYS_NTSC,
//     Clock::ACIA_NORMAL,
//     Clock::DISABLED,
//     1,
//     {0, 0, 255},
//     false
// };

const MachineSettings* const machineSettings[] PROGMEM {
    &machinePal,
    &machinePalJiffyDos,
    &machinePalJiffyDosFastAcia,
    &machinePalJiffyDosMidiHack,
    // &machineNtsc
};

// Available actions
void setLightingMode (const LightingMode newMode);
void setAnimation (const int newAnimation);
void setBrightness (const int8_t diff);
void reset ();
void setMachineConfiguration (const byte newConfiguration);

const HotKey hotKeys[] = {
    {C16Key::DEL, []{reset ();}},

    {C16Key::COMMA, []{setAnimation (0);}},
    {C16Key::PERIOD, []{setAnimation (1);}},

    {C16Key::_1,[]{setLightingMode (LightingMode::ALWAYS_ON);}},
    {C16Key::_2,[]{setLightingMode (LightingMode::ALWAYS_OFF);}},
    {C16Key::_3,[]{setLightingMode (LightingMode::PRESSED_OFF);}},
    {C16Key::_4,[]{setLightingMode (LightingMode::PRESSED_ON);}},

    {C16Key::PLUS, [] {setBrightness(+1);}},
    {C16Key::MINUS, [] {setBrightness(-1);}},

    {C16Key::F1, [] {setMachineConfiguration (0);}},
    {C16Key::F2, [] {setMachineConfiguration (1);}},
    {C16Key::F3, [] {setMachineConfiguration (2);}},
    {C16Key::HELP, [] {setMachineConfiguration (3);}},
};
