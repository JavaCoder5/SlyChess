#include "generatePseudoLegalMoves.h"

void generatePseudoLegalMoves(Move(*moves)[], bool sideToMove)
{
    U64 ownPieces = sideToMove ? allWhiteBB : allBlackBB;
    U64 opponentPieces = sideToMove ? allBlackBB : allWhiteBB;

    int movesPointer = 0; // Index to track where to insert moves in the moves array

    // Generate moves for each piece type and add to the moves array
    // Generate pawn moves:

    U64 pawnBBCopy = sideToMove ? wPawnBB : bPawnBB;
    while (true)
    {
        int sq = count_trailing_zeros(pawnBBCopy);
        if (sq == 64) break; // No more pawns

        U64 attacksCopy = sideToMove ? wPawnAttacks[sq] & ~allPiecesBB : bPawnAttacks[sq] & ~allPiecesBB;

        while (true)
        {
            U64 promotionMoves = attacksCopy & (sideToMove ? whitePromotionMask : blackPromotionMask);
            while (promotionMoves)
            {
                int promotosq = count_trailing_zeros(promotionMoves);
                (*moves)[movesPointer] = 0x0 | (sq) | (promotosq << 6) | FLAG_PROMOTION_Q;
                movesPointer++;
                (*moves)[movesPointer] = 0x0 | (sq) | (promotosq << 6) | FLAG_PROMOTION_R;
                movesPointer++;
                (*moves)[movesPointer] = 0x0 | (sq) | (promotosq << 6) | FLAG_PROMOTION_N;
                movesPointer++;
                (*moves)[movesPointer] = 0x0 | (sq) | (promotosq << 6) | FLAG_PROMOTION_B;
                movesPointer++;

                U64 bitMask = (1ULL << promotosq);
                attacksCopy = attacksCopy & ~bitMask; // Clear the most significant bit
                promotionMoves = promotionMoves & ~bitMask; // Clear the most significant bit
            }

            int tosq = count_trailing_zeros(attacksCopy);
            if (tosq == 64) break; // No more moves for this pawn

            (*moves)[movesPointer] = 0x0 | (sq) | (tosq << 6);

            movesPointer++;

            U64 bitMask = (1ULL << tosq);
            attacksCopy = attacksCopy & ~bitMask; // Clear the most significant bit
        }
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
                capturesCopy = capturesCopy & ~bitMask; // Clear the most significant bit
                promotionCaptures = promotionCaptures & ~bitMask; // Clear the most significant bit
            }

            int tosq = count_trailing_zeros(capturesCopy);
            if (tosq == 64) break; // No more captures for this pawn

            // Handle promotion captures


            (*moves)[movesPointer] = 0x0 | (sq) | (tosq << 6);

            movesPointer++;

            U64 bitMask = (1ULL << tosq);
            capturesCopy = capturesCopy & ~bitMask; // Clear the most significant bit
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

		U64 attacksCopy = knightAttacks[sq] & ~ownPieces;

        while (true)
        {
			int tosq = count_trailing_zeros(attacksCopy);
			if (tosq == 64) break; // No more moves for this knight

			(*moves)[movesPointer] = 0x0 | (sq) | (tosq << 6);
			movesPointer++;

			U64 bitMask = (1ULL << tosq);
			attacksCopy = attacksCopy & ~bitMask; // Clear the most significant bit
        }

        U64 bitMask = (1ULL << sq);
		knightBBCopy = knightBBCopy & ~bitMask; // Clear the most significant bit

    }

    // Generate bishop moves
	U64 bishopBBCopy = sideToMove ? wBishopBB : bBishopBB;

    while (true)
    {
		int sq = count_trailing_zeros(bishopBBCopy);
		if (sq == 64) break; // No more bishops

        U64 attacksCopy = lookupBishopAttacks(sq, allPiecesBB) & ~ownPieces;

        while (true)
        {
			int tosq = count_trailing_zeros(attacksCopy);
			if (tosq == 64) break; // No more moves for this bishop

            (*moves)[movesPointer] = 0x0 | (sq) | (tosq << 6);
            movesPointer++;

			U64 bitMask = (1ULL << tosq);
			attacksCopy = attacksCopy & ~bitMask; // Clear the most significant bit
        }

		U64 bitmask = (1ULL << sq);
		bishopBBCopy = bishopBBCopy & ~bitmask; // Clear the most significant bit

    }

    // Generate rook moves
    U64 rookBBCopy = sideToMove ? wRookBB : bRookBB;

    while (true)
    {
        int sq = count_trailing_zeros(rookBBCopy);
        if (sq == 64) break; // No more bishops

        U64 attacksCopy = lookupRookAttacks(sq, allPiecesBB) & ~ownPieces;

        while (true)
        {
            int tosq = count_trailing_zeros(attacksCopy);
            if (tosq == 64) break; // No more moves for this bishop

            (*moves)[movesPointer] = 0x0 | (sq) | (tosq << 6);
            movesPointer++;

            U64 bitMask = (1ULL << tosq);
            attacksCopy = attacksCopy & ~bitMask; // Clear the most significant bit
        }

        U64 bitmask = (1ULL << sq);
        rookBBCopy = rookBBCopy & ~bitmask; // Clear the most significant bit

    }


}