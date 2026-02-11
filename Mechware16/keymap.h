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

#include "common.h"
#include "C16Key.h"
#include "MatrixCoordinates.h"

class KeyMap {
public:
    KeyMap () = default;
    ~KeyMap () = default;

    boolean begin ();

    C16Key getKey (byte row, byte col) const;

    const char* getKeyName (C16Key key) const;

    MatrixCoordinates getCoordinates (C16Key key) const;

private:
    static const C16Key KEYMAP[MATRIX_ROWS][MATRIX_COLS] PROGMEM;

    static const char KEY_NAMES[MATRIX_ROWS][MATRIX_COLS][4];

    /** \brief Map a key to the (row, col) tuple describing its position in the keyboard matrix
     *
     * Useful for quick lookups.
     *
     * Built by buildKeyCoordinates().
     */
    MatrixCoordinates keyCoordinates[N_PHYSICAL_KEYS];

    /** \brief Populate the keyCoordinates array
     */
    boolean buildKeyCoordinates ();
};
