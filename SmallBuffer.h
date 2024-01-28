/**
 * Copyright (c) 2024 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
 
/**
 * \file SmallBuffer.h
 * \author SukkoPera <software@sukkology.net>
 * \date 22 Feb 2020
 * \brief General-Purpose Buffer
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MechBoard16
 */

#pragma once

#include <Arduino.h>

template <typename T, byte SIZE>
class SmallBuffer {
public:
	T buf[SIZE];

	byte size;
	
	void begin () {
		size = 0;
	}

	const T& operator [](const byte i) const {
		return buf[i];
	}

	boolean full () const {
		return size >= SIZE;		// Better be safe ;)
	}

	int8_t find (const T k) const {
		int8_t ret = -1;
	
		for (byte i = 0; i < size; ++i) {
			if (buf[i] == k) {
				ret = i;
				break;
			}
		}
	
		return ret;
	}

	inline boolean append (const T k) {
		boolean spaceAvailable = !full ();
		if (spaceAvailable) {
			buf[size++] = k;
		}
	
		return spaceAvailable;
	}

	inline boolean remove (const T k) {
		int8_t pos = find (k);
		boolean found = pos >= 0;
	
		if (found) {
			for (byte i = pos; i < size - 1; ++i) {
				buf[i] = buf[i + 1];
			}

			--size;
		}
	
		return found;
	}
};
