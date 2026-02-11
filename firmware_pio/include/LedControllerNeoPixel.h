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

#ifdef ENABLE_LEDCONTROLLER_NEOPIXEL

#include "LedController.h"
#include <Adafruit_NeoPixel.h>


// Same as Adafruit_NeoPixel::Color() but constexpr
static constexpr uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

class LedControllerNeoPixel: public LedController {
public:
    LedControllerNeoPixel ();
    ~LedControllerNeoPixel () = default;

    boolean begin () override;
    void setLedForKey (C16Key key, bool on) override;
    void setAllLeds (bool on) override;
    void setBrightness (byte brightness) override;

private:
    static constexpr uint32_t COLOR_ON = Color(0, 255, 0);
    static constexpr uint32_t COLOR_OFF = Color(255, 255, 0);

    Adafruit_NeoPixel pixels;

    byte getPixelIndex (C16Key key) const;
};

#endif
