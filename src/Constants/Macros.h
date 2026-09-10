#pragma once
#include "Constants.h"

#if defined(_MSC_VER)
// MSVC Compiler Setup
#include <intrin.h>
// Prefer TZCNT when available: it maps directly to a single instruction that returns 64 for zero inputs.
#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(_tzcnt_u64)

inline int count_trailing_zeros(U64 mask) {
    // Use TZCNT which returns 64 for a zero input on x86_64 CPUs supporting BMI1.
    // Fall back to BitScanForward if needed by checking mask==0 to preserve semantics.
    if (mask == 0) return 64;
    return static_cast<int>(_tzcnt_u64(mask));
}

#pragma intrinsic(__popcnt64)
inline int mask_popcount(U64 mask) {
    return static_cast<int>(__popcnt64(mask));
}
#elif defined(__GNUC__) || defined(__clang__)
// GCC or Clang Compiler Setup
inline int count_trailing_zeros(uint64_t mask) {
    // __builtin_ctzll behavior is undefined if mask is 0
    return mask ? __builtin_ctzll(mask) : 64;
}

inline int mask_popcount(uint64_t mask) {
    return __builtin_popcountll(mask);
}
#else
// Generic fallback for any other compiler
inline int count_trailing_zeros(U64 mask) {
    if (mask == 0) return 64;
    int count = 0;
    while ((mask & 1) == 0) {
        mask >>= 1;
        count++;
    }
    return count;
}

inline int mask_popcount(U64 mask) {
    int count = 0;
    uint64_t temp = mask;
    while (temp) {
        temp &= (temp - 1);
        count++;
    }
    return count;
}
#endif