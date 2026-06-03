#include "initSliders.h"

// Global magic structures for runtime lookups
std::array<Magic, 64> RookTableConfig;
std::array<Magic, 64> BishopTableConfig;

// Giant, memory-compact flat arrays to store attack answers
std::array<U64, 102400> RookAttackTable;
std::array<U64, 5248> BishopAttackTable;

static U64 generateRookMask(int square) {
    U64 mask = 0ULL;
    int tr = square / 8; // Rank (0-7)
    int tf = square % 8; // File (0-7)

    // Vertical Ray: Up (Must go up to 6, stopping before rank 7)
    for (int r = tr + 1; r <= 6; ++r) mask |= (1ULL << (r * 8 + tf));

    // Vertical Ray: Down (Must go down to 1, stopping before rank 0)
    for (int r = tr - 1; r >= 1; --r) mask |= (1ULL << (r * 8 + tf));

    // Horizontal Ray: Right (Must go up to 6, stopping before file 7)
    for (int f = tf + 1; f <= 6; ++f) mask |= (1ULL << (tr * 8 + f));

    // Horizontal Ray: Left (Must go down to 1, stopping before file 0)
    for (int f = tf - 1; f >= 1; --f) mask |= (1ULL << (tr * 8 + f));

    return mask;
}

static U64 generateBishopMask(int square) {
    U64 mask = 0ULL;
    int tr = square / 8, tf = square % 8;

    // Up-Right
    for (int r = tr + 1, f = tf + 1; r <= 6 && f <= 6; ++r, ++f) mask |= (1ULL << (r * 8 + f));
    // Up-Left
    for (int r = tr + 1, f = tf - 1; r <= 6 && f >= 1; ++r, --f) mask |= (1ULL << (r * 8 + f));
    // Down-Right
    for (int r = tr - 1, f = tf + 1; r >= 1 && f <= 6; --r, ++f) mask |= (1ULL << (r * 8 + f));
    // Down-Left
    for (int r = tr - 1, f = tf - 1; r >= 1 && f >= 1; --r, --f) mask |= (1ULL << (r * 8 + f));

    return mask;
}

static U64 computeRookAttacksOTF(int square, U64 blockers) {
    U64 attacks = 0ULL;
    int tr = square / 8, tf = square % 8;
    int dr[] = { 1, -1, 0, 0 }, df[] = { 0, 0, 1, -1 };

    for (int d = 0; d < 4; ++d) {
        for (int r = tr + dr[d], f = tf + df[d]; r >= 0 && r < 8 && f >= 0 && f < 8; r += dr[d], f += df[d]) {
            U64 bit = (1ULL << (r * 8 + f));
            attacks |= bit;
            if (blockers & bit) break; // Trailing rays are blocked!
        }
    }
    return attacks;
}

static U64 computeBishopAttacksOTF(int square, U64 blockers) {
    U64 attacks = 0ULL;
    int tr = square / 8, tf = square % 8;
    int dr[] = { 1, 1, -1, -1 }, df[] = { 1, -1, 1, -1 };

    for (int d = 0; d < 4; ++d) {
        for (int r = tr + dr[d], f = tf + df[d]; r >= 0 && r < 8 && f >= 0 && f < 8; r += dr[d], f += df[d]) {
            U64 bit = (1ULL << (r * 8 + f));
            attacks |= bit;
            if (blockers & bit) break; // Path blocked!
        }
    }
    return attacks;
}

static U64 generateBlockerPermutation(int index, int bits_in_mask, U64 mask) {
    // This maps the integer 'index' to the scattered bits of 'mask'
    U64 blockers = 0ULL;
    int bit_count = 0;

    for (int square = 0; square < 64; ++square) {
        if (mask & (1ULL << square)) {
            if (index & (1 << bit_count)) {
                blockers |= (1ULL << square);
            }
            bit_count++;
        }
    }
    return blockers;
}

static U64 findMagicNumber(int square, int bits, bool is_rook) {
    U64 mask = is_rook ? generateRookMask(square) : generateBishopMask(square);
    int combos = 1 << bits;

    // Create local arrays of the permutations and their correct attacks
    std::vector<U64> blockers(combos);
    std::vector<U64> attacks(combos);
    for (int i = 0; i < combos; ++i) {
        blockers[i] = generateBlockerPermutation(i, bits, mask);
        attacks[i] = is_rook ? computeRookAttacksOTF(square, blockers[i])
            : computeBishopAttacksOTF(square, blockers[i]);
    }

    // Try random numbers until we find one with ZERO harmful collisions
    std::vector<U64> test_table(combos);
    std::vector<U64> epoch(combos, 0);
    U64 current_epoch = 0;

    while (true) {
        // Generate a candidate magic (sparse random numbers work best)
        U64 candidate = random_U64() & random_U64() & random_U64();
        int shift = 64 - bits;
        bool success = true;
        current_epoch++;

        for (int i = 0; i < combos; ++i) {
            int idx = (blockers[i] * candidate) >> shift;

            // Check for collision within this trial run
            if (epoch[idx] == current_epoch) {
                if (test_table[idx] != attacks[i]) {
                    success = false; // HARMFUL COLLISION! Try next magic.
                    break;
                }
            }
            else {
                epoch[idx] = current_epoch;
                test_table[idx] = attacks[i];
            }
        }

        if (success) return candidate; // Found one!
    }
}

static void initRooks() {
    int rook_offset = 0;

    // Use a secondary flag array to keep track of which slots have been written to
    // 0 means unvisited, 1 means populated
    static std::array<uint8_t, 102400> rook_slot_visited = { 0 };

    for (int sq = 0; sq < 64; ++sq) {
        RookTableConfig[sq].mask = generateRookMask(sq);
        RookTableConfig[sq].shift = 64 - RookBits[sq];
        // Dynamically find a magic that is 100% guaranteed to work with your code:
        RookTableConfig[sq].magic = findMagicNumber(sq, RookBits[sq], true);
        RookTableConfig[sq].ptr = &RookAttackTable[rook_offset];

        int calculated_bits = mask_popcount(RookTableConfig[sq].mask);
        if (calculated_bits != RookBits[sq]) {
            std::cout << "SQUARE " << sq << " MISMATCH! Mask has " << calculated_bits << " bits, but RookBits says " << RookBits[sq] << "\n";
        }

        int rook_combos = 1 << RookBits[sq];
        for (int i = 0; i < rook_combos; ++i) {
            U64 blockers = generateBlockerPermutation(i, RookBits[sq], RookTableConfig[sq].mask);
            int magic_idx = (blockers * RookTableConfig[sq].magic) >> RookTableConfig[sq].shift;

            int absolute_index = rook_offset + magic_idx;
            U64 computed_attacks = computeRookAttacksOTF(sq, blockers);

            int actual_bits = mask_popcount(RookTableConfig[sq].mask);
            if (actual_bits != RookBits[sq]) {
                std::cerr << "Mismatch on square " << sq
                    << ": Mask has " << actual_bits
                    << " bits, but RookBits says " << RookBits[sq] << "\n";
            }


            // Write the data and mark the slot as visited
            RookAttackTable[absolute_index] = computed_attacks;
            rook_slot_visited[absolute_index] = 1;
        }

        rook_offset += rook_combos;
    }
}

static void initBishops() {
    int bishop_offset = 0;
    static std::array<uint8_t, 5248> bishop_slot_visited = { 0 };

    for (int sq = 0; sq < 64; ++sq) {
        // 1. Generate the correct inner mask
        BishopTableConfig[sq].mask = generateBishopMask(sq);

        // 2. Calculate the shift
        BishopTableConfig[sq].shift = 64 - BishopBits[sq];

        // 3. Dynamically find a 100% working magic number for this layout
        BishopTableConfig[sq].magic = findMagicNumber(sq, BishopBits[sq], false);

        // 4. Map the pointer to our flat array
        BishopTableConfig[sq].ptr = &BishopAttackTable[bishop_offset];

        int bishop_combos = 1 << BishopBits[sq];
        for (int i = 0; i < bishop_combos; ++i) {
            U64 blockers = generateBlockerPermutation(i, BishopBits[sq], BishopTableConfig[sq].mask);
            int magic_idx = (blockers * BishopTableConfig[sq].magic) >> BishopTableConfig[sq].shift;

            int absolute_index = bishop_offset + magic_idx;
            U64 computed_attacks = computeBishopAttacksOTF(sq, blockers);

            // DEBUG CHECK: Ensure zero harmful collisions
            if (bishop_slot_visited[absolute_index] == 1) {
                if (BishopAttackTable[absolute_index] != computed_attacks) {
                    std::cerr << "[CRITICAL ERROR] Harmful Bishop Magic Collision Detected!\n"
                        << "  Square: " << sq << "\n"
                        << "  Absolute Index: " << absolute_index << "\n"
                        << std::flush;
                }
            }

            BishopAttackTable[absolute_index] = computed_attacks;
            bishop_slot_visited[absolute_index] = 1;
        }

        bishop_offset += bishop_combos;
    }
}

void initSliders() {
    initRooks();
    initBishops();
}