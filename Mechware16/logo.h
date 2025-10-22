/**
 * Copyright (c) 2024 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
 
/**
 * \file logo.h
 * \author SukkoPera <software@sukkology.net>
 * \date 22 Feb 2020
 * \brief Program logo
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MechBoard16
 */

#pragma once

const char logo[] PROGMEM = "___  ___          _    ______                     _  __    ____\n"
                   "|  \\/  |         | |   | ___ \\                   | |/  |  / ___|\n"
                   "| .  . | ___  ___| |__ | |_/ / ___   __ _ _ __ __| |`| | / /___\n"
                   "| |\\/| |/ _ \\/ __| '_ \\| ___ \\/ _ \\ / _` | '__/ _` | | | | ___ \\\n"
                   "| |  | |  __/ (__| | | | |_/ / (_) | (_| | | | (_| |_| |_| \\_/ |\n"
                   "\\_|  |_/\\___|\\___|_| |_\\____/ \\___/ \\__,_|_|  \\__,_|\\___/\\_____/\n";

#define PSTR_TO_F(s) reinterpret_cast<const __FlashStringHelper *> (s)
