/**
 * Copyright (c) 2024 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
 
/**
 * \file config.h
 * \author SukkoPera <software@sukkology.net>
 * \date 22 Feb 2020
 * \brief Configuration File
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MechBoard16
 */

#pragma once

/** \def ENABLE_DEVELOPER_MODE
 *
 * \brief Enable developer mode
 *
 * This currently waits for the serial port to be opened at startup and has
 * debug messages enabled by default.
 */
#define ENABLE_DEVELOPER_MODE

/** \def DISABLE_LOGGING
 *
 * \brief Fully disable logging to the serial port
 * 
 * This also reduces the project size.
 */
//~ #define DISABLE_LOGGING

/** \def THRESHOLD_LEVEL
 *
 * \brief Threshold log level
 *
 * If this is defined, all messages below the threshold will NOT be compiled in
 * the binary and will not show up even if setLevel() is used at runtime.
 * 
 * Useful to strip debug messages from releases.
 */
//~ #define THRESHOLD_LEVEL LOG_LEVEL_INFO

/** \def ENABLE_SERIAL_COMMANDS
 *
 * \brief Enable support for receiving some commands from the serial (CDC) port
 */
//~ #define ENABLE_SERIAL_COMMANDS

//~ #define ENABLE_MATRIX_DEBUG

/** \brief Keyboard poll/report interval (ms)
 *
 * Like #CONTROLLER_READ_INTERVAL_MS but for the keyboard ;).
 * 
 * This is important because it seems that reports get dropped if we send them
 * too often. I have no idea if this is due to the HID library, MCU, USB/HID
 * protocol or whatever. Symptoms are keys getting stuck as if they were
 * constantly pressed, probably because a "release" report was sent too soon
 * after the "press" one.
 */
const unsigned long KEYBOARD_SCAN_INTERVAL_MS = 15;

/** \brief Debounce factor for the C16 keyboard
 *
 * All mechanical switches exhibit a "bouncing" phenomenon, that must be treated
 * appropriately when they are read.
 * 
 * This is the number of consecutive line samplings that must return the same
 * value for a key to be considered pressed/released. Increase it if keys get
 * pressed briefly a second time after they are released.
 *
 * C16 keyboards seem to bounce just a bit, maybe because they are newer, so
 * this does not need to be too high.
 */
#define DEBOUNCE_FACTOR_C16 20

/*! \brief Retry failed key presses/releases
 *
 * Enabling this can cause a mess, as if a key isn't mapped in the current
 * layout, keypresses will fail and the code will loop indefinitely. No point in
 * enabling this at the moment.
 */
#define PEDANTIC_PRESS_RELEASE_CHECKS

/** \def ENABLE_EURO_KEY
 *
 * \brief Replace Pound sign with Euro sign
 * 
 * This makes the "Sterling Pound" key on VIC20/C64/C16/+4 keyboards display the
 * Euro symbol instead (when in symbolic mapping).
 */
//~ #define ENABLE_EURO_KEY

#define KEYMAPS_IN_FLASH

//~ #define LED_PIN LED_BUILTIN

//! \brief Maximum length of command strings sent on the serial port
const byte MAX_SERCMD_LEN = 32;

/*******************************************************************************
 * END OF SETTINGS
 ******************************************************************************/


#include <Arduino.h>

//! \name Version number stuff
//! @{
#define MECH16_VERSION_MAJOR 0
#define MECH16_VERSION_MINOR 1
#define MECH16_VERSION_PATCH 0

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define MECH16_VERSION_STR STR(MECH16_VERSION_MAJOR) "." STR(MECH16_VERSION_MINOR) "." STR(MECH16_VERSION_PATCH)
#define MECH16_BUILDTIME_STR (__DATE__ " " __TIME__)

//! @}
