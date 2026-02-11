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

#include "keymap.h"

const C16Key KeyMap::KEYMAP[MATRIX_ROWS][MATRIX_COLS] PROGMEM = {
    {C16Key::DEL,  C16Key::RETURN,   C16Key::POUND,     C16Key::HELP,  C16Key::F1,     C16Key::F2,    C16Key::F3,    C16Key::AT},
    {C16Key::_3,   C16Key::W,        C16Key::A,         C16Key::_4,    C16Key::Z,      C16Key::S,     C16Key::E,     C16Key::SHIFT},
    {C16Key::_5,   C16Key::R,        C16Key::D,         C16Key::_6,    C16Key::C,      C16Key::F,     C16Key::T,     C16Key::X},
    {C16Key::_7,   C16Key::Y,        C16Key::G,         C16Key::_8,    C16Key::B,      C16Key::H,     C16Key::U,     C16Key::V},
    {C16Key::_9,   C16Key::I,        C16Key::J,         C16Key::_0,    C16Key::M,      C16Key::K,     C16Key::O,     C16Key::N},
    {C16Key::DOWN, C16Key::P,        C16Key::L,         C16Key::UP,    C16Key::PERIOD, C16Key::COLON, C16Key::MINUS, C16Key::COMMA},
    {C16Key::LEFT, C16Key::ASTERISK, C16Key::SEMICOLON, C16Key::RIGHT, C16Key::ESC,    C16Key::EQUAL, C16Key::PLUS,  C16Key::SLASH},
    {C16Key::_1,   C16Key::CLEAR,    C16Key::CTRL,      C16Key::_2,    C16Key::SPACE,  C16Key::CMD,   C16Key::Q,     C16Key::RUNSTOP}
};

// Only useful for debugging
constexpr char KeyMap::KEY_NAMES[MATRIX_ROWS][MATRIX_COLS][4] = {
    {"DEL", "RET", "£",   "HLP", "F1",  "F2", "F3", "@"},
    {"3",   "W",   "A",   "4",   "Z",   "S",  "E",  "SHF"},
    {"5",   "R",   "D",   "6",   "C",   "F",  "T",  "X"},
    {"7",   "Y",   "G",   "8",   "B",   "H",  "U",  "V"},
    {"9",   "I",   "J",   "0",   "M",   "K",  "O",  "N"},
    {"DN",  "P",   "L",   "UP",  ".",   ":",  "-",  ","},
    {"LF",  "*",   ";",   "RT",  "ESC", "=",  "+",  "/"},
    {"1",   "CLR", "CTL", "2",   "SPC", "C=", "Q",  "RUN"}
};

boolean KeyMap::begin () {
    return buildKeyCoordinates ();
}

C16Key KeyMap::getKey(byte row, byte col) const {
    const byte k = pgm_read_byte (&KEYMAP[row][col]);
    return static_cast<C16Key> (k);
}

const char* KeyMap::getKeyName(C16Key key) const {
    MatrixCoordinates coordinates = getCoordinates (key);
    return KEY_NAMES[coordinates.row][coordinates.col];
}

MatrixCoordinates KeyMap::getCoordinates(C16Key key) const {
    return keyCoordinates[static_cast<byte> (key)];
}

boolean KeyMap::buildKeyCoordinates() {
    bool found;

    for (byte i = 0; i < N_PHYSICAL_KEYS; ++i) {
        found = false;
        for (byte row = 0; row < MATRIX_ROWS && !found; ++row) {
            for (byte col = 0; col < MATRIX_COLS && !found; ++col) {
                if (pgm_read_byte (&KEYMAP[row][col]) == i) {
                    keyCoordinates[i].row = row;
                    keyCoordinates[i].col = col;
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

