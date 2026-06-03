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