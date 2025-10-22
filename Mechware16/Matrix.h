/**
 * Copyright (c) 2024 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
 
/**
 * \file Matrix.h
 * \author SukkoPera <software@sukkology.net>
 * \date 22 Feb 2020
 * \brief Generic Keyboard Matrix
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MechBoard16
 */

#ifndef MATRIX_H_
#define MATRIX_H_

// This is a C++11 "Alias Declaration"
template <byte NUMROWS, typename TYPECOLS>
using MatrixBase = TYPECOLS[NUMROWS];

#endif
