#pragma once
#include <array>
#include <cstdint>

using U64 = unsigned long long;
using I16 = short;
using U8 = uint8_t;
using Move = unsigned short;

enum searchMode {searchForDepth, searchForTimeControl, searchForSetTime};

// Compact undo information per move (no full snapshots)
struct UndoInfo {
    Move move;
    char capturedPiece; // 'P','N','B','R','Q','K' for white, lowercase for black, 0 if none
    int capturedSquare; // square of captured piece (for en-passant may differ from to-square)
    int prevEnPassantSquare;
    bool prev_wKR, prev_wQR, prev_bKR, prev_bQR;
    bool wasEnPassant;
    bool wasPromotion;
    bool wasCastle;
    int rookFrom, rookTo;
};

constexpr int MAX_UNDO = 8192;

static const int INF = 1000000000;
static const int MINF = -1000000000;

constexpr int MATE = -100000;

#define WHITE true
#define BLACK false

const U64 WPAWN_START = 0x000000000000FF00ULL;
const U64 WKNIGHT_START = 0x0000000000000042ULL;
const U64 WBISHOP_START = 0x0000000000000024ULL;
const U64 WROOK_START = 0x0000000000000081ULL;
const U64 WQUEEN_START = 0x0000000000000010ULL;
const U64 WKING_START = 0x0000000000000008ULL;

const U64 BPAWN_START = 0x00FF000000000000ULL;
const U64 BKNIGHT_START = 0x4200000000000000ULL;
const U64 BBISHOP_START = 0x2400000000000000ULL;
const U64 BROOK_START = 0x8100000000000000ULL;
const U64 BQUEEN_START = 0x1000000000000000ULL;
const U64 BKING_START = 0x0800000000000000ULL;


// Shifting counts per square (0 to 63)
inline constexpr std::array<int, 64> RookBits = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

inline constexpr std::array<int, 64> BishopBits = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

struct Magic {
    const U64* ptr; // Points to the start of this square's block in the flat array
    U64 mask;       // Inner ray blocker mask
    U64 magic;      // Hardcoded magic multiplier
    int shift;      // 64 - Bits
};

// Move structure: 16 bits total (short)
// Bits 0-5: from square (0-63)
// Bits 6-11: to square (0-63)
// Bits 12-15: special flags (promotion, castling, en passant, etc.)
#define FLAG_PROMOTION_Q (0x1 << 12)
#define FLAG_PROMOTION_R (0x2 << 12)
#define FLAG_PROMOTION_B (0x3 << 12)
#define FLAG_PROMOTION_N (0x4 << 12)
#define FLAG_CASTLE_K (0x8 << 12)
#define FLAG_CASTLE_Q (0x9 << 12)
#define FLAG_EN_PASSANT (0xA << 12)

const inline U64 whitePromotionMask = 0xFF00000000000000ULL;
const inline U64 blackPromotionMask = 0x00000000000000FFULL;

struct TTEntry
{
    U64  key;
    Move move;
    int  score;
    U8   depth;
    U8   flag;
};

constexpr int TT_SIZE = 1 << 22;
constexpr int TT_MASK = TT_SIZE - 1;

#define TT_NONE  0
#define TT_EXACT 1
#define TT_ALPHA 2
#define TT_BETA  3

enum Pieces {
    wPawn,
    wKnight,
    wBishop,
    wRook,
    wQueen,
    wKing,
    bPawn,
    bKnight,
    bBishop,
    bRook,
    bQueen,
    bKing
};

inline constexpr int MAX_DEPTH = 99;