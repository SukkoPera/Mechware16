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

#ifdef ENABLE_LEDCONTROLLER_MAX7221

#include "common.h"
#include "keymap.h"
#include "LedControllerMax7221.h"

extern KeyMap keyMap;

LedControllerMax7221::LedControllerMax7221():  lc (PIN_MAX7221_DATA, PIN_MAX7221_CLK, PIN_MAX7221_SEL, 1 /* Number of MAX72xx chips */) {
}

boolean LedControllerMax7221::begin () {
    // Note the MAX72XX might show a random pattern at startup, so let's clear it ASAP
    lc.shutdown (0, false);
    lc.clearDisplay (0);

    return buildLedCoordinates ();
}

void LedControllerMax7221::setLedForKey (C16Key key, bool on) {
    if (key != C16Key::NONE) {
        const MatrixCoordinates& pos = ledCoordinates[static_cast<byte> (key)];
        lc.setLed (0, pos.row, pos.col, on);
    }
}

void LedControllerMax7221::setAllLeds (bool on) {
    for (byte i = 0; i < MATRIX_COLS; ++i) {
        lc.setRow (0, i, on ? 0xFF : 0x00);
    }
}

void LedControllerMax7221::setBrightness (byte brightness) {
    lc.setIntensity (0, brightness);
}

// Populates the ledCoordinates array
boolean LedControllerMax7221::buildLedCoordinates () {
    bool found;

    for (byte i = 0; i < N_PHYSICAL_KEYS; ++i) {
        auto key = static_cast<C16Key> (i);
        found = false;
        for (byte row = 0; row < MATRIX_ROWS && !found; ++row) {
            for (byte col = 0; col < MATRIX_COLS && !found; ++col) {
                if (keyMap.getKey(row, col) == key) {
                    /* The led matrix was supposed to be the same as the keyboard matrix. I don't know whether I made a
                     * wiring mistake or if LedControl numbers things differently, but it turns out we need to swap the
                     * coordinates and modify them slightly in order to use them with lc.setLed().
                     */
                    ledCoordinates[i].row = col;
                    ledCoordinates[i].col = (row + 1) % 8;
                    found = true;
                }
            }
        }

        if (!found) {
            break;
        }
    }

    return found;
}

#endif
