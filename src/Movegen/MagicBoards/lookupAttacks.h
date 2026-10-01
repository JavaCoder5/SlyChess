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

// Global magic structures for runtime lookups
extern std::array<Magic, 64> RookTableConfig;
extern std::array<Magic, 64> BishopTableConfig;

// Giant, memory-compact flat arrays to store attack answers
extern std::array<U64, 102400> RookAttackTable;
extern std::array<U64, 5248> BishopAttackTable;

inline U64 lookupRookAttacks(int square, U64 occupancy) {
    U64 blockers = occupancy & RookTableConfig[square].mask;
    int index = (blockers * RookTableConfig[square].magic) >> RookTableConfig[square].shift;
    return RookTableConfig[square].ptr[index];
}

inline U64 lookupBishopAttacks(int square, U64 occupancy) {
    U64 blockers = occupancy & BishopTableConfig[square].mask;
    int index = (blockers * BishopTableConfig[square].magic) >> BishopTableConfig[square].shift;
    return BishopTableConfig[square].ptr[index];
}

inline U64 lookupQueenAttacks(int square, U64 occupancy) {
    return lookupRookAttacks(square, occupancy) | lookupBishopAttacks(square, occupancy);
}