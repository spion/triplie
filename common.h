/*
 *  Copyright (C) Gorgi Kosev a.k.a. Spion, John Peterson.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#pragma once
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <limits>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
using namespace std;

typedef	uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

extern u8 verb;

template<class T> T lmax(T) {
	return numeric_limits<T>::max();
}
template<class T> T lmin(T) {
	return numeric_limits<T>::min();
}
template<class T> void range_lim(T& v, T l = numeric_limits<T>::min(), T u = numeric_limits<T>::max()) {
	if (v < l) v = l;
	if (v > u) v = u;
}
void log(const char* f, ...);
void log1(const char* f, ...);
void log2(const char* f, ...);
string format(const char* f, ...);
double seconds();
string time_hm();string format_bytes(s64 bytes);
off_t fsize(string f);