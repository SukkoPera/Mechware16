/**
 * Copyright (c) 2024-2025 SukkoPera <software@sukkology.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <Arduino.h>

/* This class is derived from code found in Arduino's SoftwareSerial library
 * and should probably be credited to Mikal Hart (http://www.arduiniana.org).
 *
 * License: LGPL 1.2
 */
template <typename T, typename DIMT, DIMT SIZE>
class CircularBuffer {
private:
	/*volatile*/ T buf[SIZE];
	volatile DIMT head;
	volatile DIMT tail;

	inline DIMT next () const {
		return (tail + 1) % SIZE;
	}

public:
	void begin () {
		head = 0;
		tail = 0;
	}

	inline boolean empty () const {
		return head == tail;
	}

	inline boolean full () const {
		return next () == head;
	}

	DIMT available () const {
		// FIXME: Probably need a cast to avoid overflowing uint8
		return (tail + SIZE - head) % SIZE;
	}

	DIMT free () const {
		return SIZE - ((tail + SIZE - head) % SIZE);
	}

	boolean put (T x) {
		boolean ret;

		if ((ret = !full ())) {
			// Save new data in buffer: tail points to where byte goes
			buf[tail] = x;
			tail = next ();
		}

		return ret;
	}

	T get () {
		T x = buf[head]; // grab next byte
		head = (head + 1) % SIZE;

		return x;
	}

	T peek () const {
		// Empty buffer?
		//~ if (!empty ())
			//~ return -1;

		// Read from "head"
		return buf[head];
	}

	T peek (const byte n) const {
		// Empty buffer?
		//~ if (!empty ())
			//~ return -1;

		// Read from "head"
		return buf[(head + n) % SIZE];
	}

	// Use wisely
	T* getPointer () const {
		return const_cast<T*> (buf + head);
	}
};
