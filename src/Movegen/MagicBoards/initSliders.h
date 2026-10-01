/*
    Copyright (C) 2026 JavaCoder5

    SlyChess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SlyChess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <src/Constants/Constants.h>
#include <random>
#include <src/Constants/Macros.h>
#include <iostream>

// A simple 64-bit random number generator
static inline U64 random_U64() {
    static std::mt19937_64 gen(1337); // Fixed seed for deterministic startup
    return gen();
}

static U64 generateRookMask(int square);

static U64 generateBishopMask(int square);

static U64 computeRookAttacksOTF(int square, U64 blockers);

static U64 computeBishopAttacksOTF(int square, U64 blockers);

static U64 generateBlockerPermutation(int index, int bits_in_mask, U64 mask);

static U64 findMagicNumber(int square, int bits, bool is_rook);

static void initRooks();

static void initBishops();

void initSliders();