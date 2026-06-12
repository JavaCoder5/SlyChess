#include "generatePseudoLegalCaptures.h"

void generatePseudoLegalCaptures(Move(*moves)[], bool sideToMove, int* size)
{
    U64 ownPieces = sideToMove ? allWhiteBB : allBlackBB;
    U64 opponentPieces = sideToMove ? allBlackBB : allWhiteBB;

    int movesPointer = 0; // Index to track where to insert moves in the moves array

    // Generate moves for each piece type and add to the moves array
    // Generate pawn moves:

    U64 pawnBBCopy = sideToMove ? wPawnBB : bPawnBB;
    while (true)
    {
        // Get the square of the least significant pawn bit and generate moves for that pawn
        int sq = count_trailing_zeros(pawnBBCopy);
        if (sq == 64) break; // No more pawns

        // Generate captures for this pawn

        U64 captureMask = sideToMove ? wPawnCaptures[sq] : bPawnCaptures[sq];

        // En-passant: if an en-passant target square exists and this pawn can capture there,
        // ensure the captured pawn is present on the expected square and add the EP move.
        if (enPassantSquare >= 0 && enPassantSquare < 64) {
            U64 epBit = 1ULL << enPassantSquare;
            if (captureMask & epBit) {
                int capturedPawnSq = sideToMove ? (enPassantSquare - 8) : (enPassantSquare + 8);
                if (capturedPawnSq >= 0 && capturedPawnSq < 64) {
                    if ((opponentPieces >> capturedPawnSq) & 1ULL) {
                        (*moves)[movesPointer] = 0x0 | (sq) | (enPassantSquare << 6) | FLAG_EN_PASSANT;
                        movesPointer++;
                    }
                }
            }
        }

        U64 capturesCopy = captureMask & opponentPieces;

        while (true)
        {
            // Handle promotion captures

            U64 promotionCaptures = capturesCopy & (sideToMove ? whitePromotionMask : blackPromotionMask);
            while (promotionCaptures)
            {
                int promotosq = count_trailing_zeros(promotionCaptures);
                (*moves)[movesPointer] = 0x0 | (sq) | (promotosq << 6) | FLAG_PROMOTION_Q;
                movesPointer++;
                (*moves)[movesPointer] = 0x0 | (sq) | (promotosq << 6) | FLAG_PROMOTION_R;
                movesPointer++;
                (*moves)[movesPointer] = 0x0 | (sq) | (promotosq << 6) | FLAG_PROMOTION_N;
                movesPointer++;
                (*moves)[movesPointer] = 0x0 | (sq) | (promotosq << 6) | FLAG_PROMOTION_B;
                movesPointer++;

                U64 bitMask = (1ULL << promotosq);
                capturesCopy = capturesCopy & ~bitMask; // Clear the least significant bit
                promotionCaptures = promotionCaptures & ~bitMask; // Clear the least significant bit
            }

            int tosq = count_trailing_zeros(capturesCopy);
            if (tosq == 64) break; // No more captures for this pawn

            (*moves)[movesPointer] = 0x0 | (sq) | (tosq << 6);

            movesPointer++;

            U64 bitMask = (1ULL << tosq);
            capturesCopy = capturesCopy & ~bitMask; // Clear the least significant bit
        }


        // Clear the bit for this pawn and continue to the next one

        U64 bitMask = (1ULL << sq);
        pawnBBCopy = pawnBBCopy & ~bitMask; // Clear the most significant bit

    }

    U64 knightBBCopy = sideToMove ? wKnightBB : bKnightBB;

    // Generate knight moves
    while (true)
    {
        int sq = count_trailing_zeros(knightBBCopy);
        if (sq == 64) break; // No more knights

        U64 attacksCopy = (knightAttacks[sq] & ~ownPieces) & opponentPieces;

        while (true)
        {
            int tosq = count_trailing_zeros(attacksCopy);
            if (tosq == 64) break; // No more moves for this knight

            if (!((1ULL << tosq) & (sideToMove ? allBlackBB : allWhiteBB)))
                continue; // Not a capture

            (*moves)[movesPointer] = 0x0 | (sq) | (tosq << 6);
            movesPointer++;

            U64 bitMask = (1ULL << tosq);
            attacksCopy = attacksCopy & ~bitMask; // Clear the least significant bit
        }

        U64 bitMask = (1ULL << sq);
        knightBBCopy = knightBBCopy & ~bitMask; // Clear the least significant bit

    }

    // Generate bishop moves
    U64 bishopBBCopy = sideToMove ? wBishopBB : bBishopBB;

    while (true)
    {
        int sq = count_trailing_zeros(bishopBBCopy);
        if (sq == 64) break; // No more bishops

        U64 attacksCopy = (lookupBishopAttacks(sq, allPiecesBB) & ~ownPieces) & opponentPieces;

        while (true)
        {
            int tosq = count_trailing_zeros(attacksCopy);
            if (tosq == 64) break; // No more moves for this bishop

            (*moves)[movesPointer] = 0x0 | (sq) | (tosq << 6);
            movesPointer++;

            U64 bitMask = (1ULL << tosq);
            attacksCopy = attacksCopy & ~bitMask; // Clear the least significant bit
        }

        U64 bitmask = (1ULL << sq);
        bishopBBCopy = bishopBBCopy & ~bitmask; // Clear the least significant bit

    }

    // Generate rook moves
    U64 rookBBCopy = sideToMove ? wRookBB : bRookBB;

    while (true)
    {
        int sq = count_trailing_zeros(rookBBCopy);
        if (sq == 64) break; // No more rooks

        U64 attacksCopy = (lookupRookAttacks(sq, allPiecesBB) & ~ownPieces) & opponentPieces;

        while (true)
        {
            int tosq = count_trailing_zeros(attacksCopy);
            if (tosq == 64) break; // No more moves for this rook

            (*moves)[movesPointer] = 0x0 | (sq) | (tosq << 6);
            movesPointer++;

            U64 bitMask = (1ULL << tosq);
            attacksCopy = attacksCopy & ~bitMask; // Clear the least significant bit
        }

        U64 bitmask = (1ULL << sq);
        rookBBCopy = rookBBCopy & ~bitmask; // Clear the least significant bit

    }

    // Generate queen moves
    U64 queenBBCopy = sideToMove ? wQueenBB : bQueenBB;

    while (true)
    {
        int sq = count_trailing_zeros(queenBBCopy);
        if (sq == 64) break; // No more queens

        U64 attacksCopy = (lookupQueenAttacks(sq, allPiecesBB) & ~ownPieces) & opponentPieces;

        while (true)
        {
            int tosq = count_trailing_zeros(attacksCopy);
            if (tosq == 64) break; // No more moves for this queen

            if (!((1ULL << tosq) & (sideToMove ? allBlackBB : allWhiteBB)))
                continue; // Not a capture

            (*moves)[movesPointer] = 0x0 | (sq) | (tosq << 6);
            movesPointer++;

            U64 bitMask = (1ULL << tosq);
            attacksCopy = attacksCopy & ~bitMask; // Clear the least significant bit
        }

        U64 bitmask = (1ULL << sq);
        queenBBCopy = queenBBCopy & ~bitmask; // Clear the least significant bit

    }

    // Generate king moves
    U64 kingBBCopy = sideToMove ? wKingBB : bKingBB;

    // Note: No loop is required since there can only ever be one king per side

    int sq = count_trailing_zeros(kingBBCopy);

    U64 kAttacksCopy = (kingAttacks[sq] & ~ownPieces) & opponentPieces;

    while (true)
    {
        int tosq = count_trailing_zeros(kAttacksCopy);
        if (tosq == 64) break; // No more moves for this king

        if (!((1ULL << tosq) & (sideToMove ? allBlackBB : allWhiteBB)))
            continue; // Not a capture

        (*moves)[movesPointer] = 0x0 | (sq) | (tosq << 6);
        movesPointer++;

        U64 bitMask = (1ULL << tosq);
        kAttacksCopy = kAttacksCopy & ~bitMask; // Clear the least significant bit
    }

    *size = movesPointer; // Set the output size to the number of moves generated
}