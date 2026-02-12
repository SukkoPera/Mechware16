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

MachineSettings machinePal {
    Clock::SYS_PAL,
    Clock::ACIA_NORMAL,
    Clock::DISABLED,
    0,
    {255, 0, 0},
    false
};

MachineSettings machinePalJiffyDos {
    Clock::SYS_PAL,
    Clock::ACIA_NORMAL,
    Clock::DISABLED,
    1,
    {127, 255, 0},
    false
};

MachineSettings machinePalFastAcia {
    Clock::SYS_PAL,
    Clock::ACIA_DOUBLE,
    Clock::DISABLED,
    0,
    {35, 255, 0},
    false
};

MachineSettings machineNtsc {
    Clock::SYS_NTSC,
    Clock::ACIA_NORMAL,
    Clock::DISABLED,
    1,
    {0, 0, 255},
    false
};

constexpr byte MACHINE_SETTINGS_NO = 4;

MachineSettings machineSettings[MACHINE_SETTINGS_NO] = {
    machinePal,
    machinePalJiffyDos,
    machinePalFastAcia,
    machineNtsc
};

// Available actions
void setLightingMode (const Mode newMode);
void setAnimation (const int newAnimation);
void setBrightness (const int8_t diff);
void reset ();
void setMachineConfiguration (const byte newConfiguration);

constexpr byte N_HOTKEYS = 13;

HotKey actions[N_HOTKEYS] = {
    {C16Key::DEL, []{reset ();}},

    {C16Key::_1, []{setAnimation (0);}},
    {C16Key::_2, []{setAnimation (1);}},

    {C16Key::F1,[]{setLightingMode (Mode::ALWAYS_ON);}},
    {C16Key::F2,[]{setLightingMode (Mode::ALWAYS_OFF);}},
    {C16Key::F3,[]{setLightingMode (Mode::PRESSED_OFF);}},
    {C16Key::HELP,[]{setLightingMode (Mode::PRESSED_ON);}},

    {C16Key::PLUS, [] {setBrightness(+1);}},
    {C16Key::MINUS, [] {setBrightness(-1);}},

    {C16Key::Q, [] {setMachineConfiguration (0);}},
    {C16Key::W, [] {setMachineConfiguration (1);}},
    {C16Key::E, [] {setMachineConfiguration (2);}},
    {C16Key::R, [] {setMachineConfiguration (3);}},
};
