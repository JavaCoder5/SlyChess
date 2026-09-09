#include "generatePseudoLegalMoves.h"

void generatePseudoLegalMoves(Move(*moves)[], bool sideToMove, int *size)
{
    U64 ownPieces = sideToMove ? allWhiteBB : allBlackBB;
    U64 opponentPieces = sideToMove ? allBlackBB : allWhiteBB;

    Move *baseOut = &(*moves)[0];
    Move *out = baseOut;

    // Precompute side-dependent references to avoid branching in hot loops
    const U64 *pawnAttacks = sideToMove ? wPawnAttacks : bPawnAttacks;
    const U64 *pawnCaptures = sideToMove ? wPawnCaptures : bPawnCaptures;
    const U64 promoMask = sideToMove ? whitePromotionMask : blackPromotionMask;

    // Generate pawn moves
    U64 pawnBBCopy = sideToMove ? wPawnBB : bPawnBB;
    while (pawnBBCopy)
    {
        U64 lsbPawn = pawnBBCopy & (0 - pawnBBCopy);
        int sq = count_trailing_zeros(lsbPawn);
        pawnBBCopy ^= lsbPawn; // clear LSB

        U64 attacksCopy = pawnAttacks[sq] & ~allPiecesBB;
        U64 attacksCompareMask = pawnAttacks[sq];
        if (attacksCompareMask != attacksCopy)
        {
            int dpSq = sq + (sideToMove ? 16 : -16);
            if (dpSq >= 0 && dpSq < 64)
                attacksCopy &= ~(1ULL << dpSq);
        }

        // promotions (non-capture)
        U64 promotionMoves = attacksCopy & promoMask;
        while (promotionMoves)
        {
            U64 lsb = promotionMoves & (0 - promotionMoves);
            int promotosq = count_trailing_zeros(lsb);
            Move baseMove = (Move)(sq | (promotosq << 6));
            *out++ = baseMove | FLAG_PROMOTION_Q;
            *out++ = baseMove | FLAG_PROMOTION_R;
            *out++ = baseMove | FLAG_PROMOTION_N;
            *out++ = baseMove | FLAG_PROMOTION_B;
            promotionMoves ^= lsb;
            attacksCopy ^= lsb;
        }

        // non-capture moves
        while (attacksCopy)
        {
            U64 lsb = attacksCopy & (0 - attacksCopy);
            int tosq = count_trailing_zeros(lsb);
            *out++ = (Move)(sq | (tosq << 6));
            attacksCopy ^= lsb;
        }

        // captures
        U64 captureMask = pawnCaptures[sq];

        // en-passant
        if (enPassantSquare >= 0 && enPassantSquare < 64)
        {
            U64 epBit = 1ULL << enPassantSquare;
            if (captureMask & epBit)
            {
                int capturedPawnSq = sideToMove ? (enPassantSquare - 8) : (enPassantSquare + 8);
                if (capturedPawnSq >= 0 && capturedPawnSq < 64)
                {
                    if ((opponentPieces >> capturedPawnSq) & 1ULL)
                    {
                        *out++ = (Move)(sq | (enPassantSquare << 6) | FLAG_EN_PASSANT);
                    }
                }
            }
        }

        U64 capturesCopy = captureMask & opponentPieces;
        // promotion captures
        U64 promotionCaptures = capturesCopy & promoMask;
        while (promotionCaptures)
        {
            U64 lsb = promotionCaptures & (0 - promotionCaptures);
            int promotosq = count_trailing_zeros(lsb);
            Move baseMove = (Move)(sq | (promotosq << 6));
            *out++ = baseMove | FLAG_PROMOTION_Q;
            *out++ = baseMove | FLAG_PROMOTION_R;
            *out++ = baseMove | FLAG_PROMOTION_N;
            *out++ = baseMove | FLAG_PROMOTION_B;
            promotionCaptures ^= lsb;
            capturesCopy ^= lsb;
        }

        while (capturesCopy)
        {
            U64 lsb = capturesCopy & (0 - capturesCopy);
            int tosq = count_trailing_zeros(lsb);
            *out++ = (Move)(sq | (tosq << 6));
            capturesCopy ^= lsb;
        }
    }

    // Knights
    U64 knightBBCopy = sideToMove ? wKnightBB : bKnightBB;
    while (knightBBCopy)
    {
        U64 lsb = knightBBCopy & (0 - knightBBCopy);
        int sq = count_trailing_zeros(lsb);
        knightBBCopy ^= lsb;

        U64 attacksCopy = knightAttacks[sq] & ~ownPieces;
        while (attacksCopy)
        {
            U64 l = attacksCopy & (0 - attacksCopy);
            int tosq = count_trailing_zeros(l);
            *out++ = (Move)(sq | (tosq << 6));
            attacksCopy ^= l;
        }
    }

    // Bishop
    U64 bishopBBCopy = sideToMove ? wBishopBB : bBishopBB;
    while (bishopBBCopy)
    {
        U64 lsb = bishopBBCopy & (0 - bishopBBCopy);
        int sq = count_trailing_zeros(lsb);
        bishopBBCopy ^= lsb;

        U64 attacksCopy = lookupBishopAttacks(sq, allPiecesBB) & ~ownPieces;
        while (attacksCopy)
        {
            U64 l = attacksCopy & (0 - attacksCopy);
            int tosq = count_trailing_zeros(l);
            *out++ = (Move)(sq | (tosq << 6));
            attacksCopy ^= l;
        }
    }

    // Rook
    U64 rookBBCopy = sideToMove ? wRookBB : bRookBB;
    while (rookBBCopy)
    {
        U64 lsb = rookBBCopy & (0 - rookBBCopy);
        int sq = count_trailing_zeros(lsb);
        rookBBCopy ^= lsb;

        U64 attacksCopy = lookupRookAttacks(sq, allPiecesBB) & ~ownPieces;
        while (attacksCopy)
        {
            U64 l = attacksCopy & (0 - attacksCopy);
            int tosq = count_trailing_zeros(l);
            *out++ = (Move)(sq | (tosq << 6));
            attacksCopy ^= l;
        }
    }

    // Queen
    U64 queenBBCopy = sideToMove ? wQueenBB : bQueenBB;
    while (queenBBCopy)
    {
        U64 lsb = queenBBCopy & (0 - queenBBCopy);
        int sq = count_trailing_zeros(lsb);
        queenBBCopy ^= lsb;

        U64 attacksCopy = lookupQueenAttacks(sq, allPiecesBB) & ~ownPieces;
        while (attacksCopy)
        {
            U64 l = attacksCopy & (0 - attacksCopy);
            int tosq = count_trailing_zeros(l);
            *out++ = (Move)(sq | (tosq << 6));
            attacksCopy ^= l;
        }
    }

    // King (single)
    U64 kingBBCopy = sideToMove ? wKingBB : bKingBB;
    int kingSq = count_trailing_zeros(kingBBCopy);
    if (kingSq != 64)
    {
        U64 kAttacksCopy = kingAttacks[kingSq] & ~ownPieces;
        while (kAttacksCopy)
        {
            U64 l = kAttacksCopy & (0 - kAttacksCopy);
            int tosq = count_trailing_zeros(l);
            *out++ = (Move)(kingSq | (tosq << 6));
            kAttacksCopy ^= l;
        }

        // Castling
        if (sideToMove ? wKingCastleKRights : bKingCastleKRights)
        {
            if ((sideToMove ? (allPiecesBB & 0x60) == 0 : (allPiecesBB & 0x6000000000000000) == 0))
                *out++ = (Move)(kingSq | ((sideToMove ? 6 : 62) << 6) | FLAG_CASTLE_K);
        }
        if (sideToMove ? wKingCastleQRights : bKingCastleQRights)
        {
            if ((sideToMove ? (allPiecesBB & 0xE) == 0 : (allPiecesBB & 0xE00000000000000) == 0))
                *out++ = (Move)(kingSq | ((sideToMove ? 2 : 58) << 6) | FLAG_CASTLE_Q);
        }
    }

    *size = (int)(out - baseOut);
}
