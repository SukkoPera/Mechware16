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

#ifdef ENABLE_LEDCONTROLLER_NEOPIXEL

#include "common.h"
#include "LedControllerNeoPixel.h"

// Order in which the LEDs are chained
constexpr C16Key LED_ORDER[N_PHYSICAL_KEYS + 4] PROGMEM = {
    C16Key::HELP, C16Key::F3, C16Key::F2, C16Key::F1,
    C16Key::DEL, C16Key::DOWN, C16Key::UP, C16Key::RIGHT, C16Key::LEFT, C16Key::_0, C16Key::_9, C16Key::_8, C16Key::_7, C16Key::_6, C16Key::_5, C16Key::_4, C16Key::_3, C16Key::_2, C16Key::_1, C16Key::ESC,
    C16Key::CTRL, C16Key::Q, C16Key::W, C16Key::E, C16Key::R, C16Key::T, C16Key::Y, C16Key::U, C16Key::I, C16Key::O, C16Key::P, C16Key::AT, C16Key::PLUS, C16Key::MINUS, C16Key::CLEAR,
    C16Key::RETURN, C16Key::ASTERISK, C16Key::SEMICOLON, C16Key::COLON, C16Key::L, C16Key::K, C16Key::J, C16Key::H, C16Key::G, C16Key::F, C16Key::D, C16Key::S, C16Key::A, C16Key::NONE /* Shift Lock */, C16Key::RUNSTOP,
    C16Key::CMD, C16Key::SHIFT, C16Key::Z, C16Key::X, C16Key::C, C16Key::V, C16Key::B, C16Key::N, C16Key::M, C16Key::COMMA, C16Key::PERIOD, C16Key::SLASH, C16Key::NONE /* Right Shift */, C16Key::POUND, C16Key::EQUAL,
    C16Key::NONE /* Space Left */, C16Key::SPACE, C16Key::NONE /* Space Right */
};

LedControllerNeoPixel::LedControllerNeoPixel(): pixels(LEDSTRIP_LEN, PIN_LEDSTRIP, NEO_GRB + NEO_KHZ800) {
}

boolean LedControllerNeoPixel::begin () {
    if (pixels.begin ()) {
        pixels.clear ();
        return true;
    } else {
        return false;
    }
}

void LedControllerNeoPixel::setLedForKey (const C16Key key, const bool on) {
    byte index = getPixelIndex (key);
    pixels.setPixelColor (index, on ? COLOR_ON : COLOR_OFF);
    pixels.show ();
}

void LedControllerNeoPixel::setAllLeds (const bool on) {
    pixels.fill (on ? COLOR_ON : COLOR_OFF);
    pixels.show ();
}

void LedControllerNeoPixel::setBrightness (const byte brightness) {
    pixels.setBrightness (brightness * 16);
    pixels.show ();
}

byte LedControllerNeoPixel::getPixelIndex (C16Key key) {
    if (key != C16Key::NONE) {      // Explicitly avoid NONE since we use it as a placeholder in LED_ORDER
        for (uint8_t i = 0; i < sizeof (LED_ORDER); i++) {
            if (LED_ORDER[i] == key) {
                return i;
            }
        }
    }

    return ~0;
}

#endif
