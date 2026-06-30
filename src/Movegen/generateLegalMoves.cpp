#include "generateLegalMoves.h"

// Generate legal moves for sideToMove by filtering pseudo-legal moves.
// Uses makeMove/unmakeMove to test each move for king safety without allocations.
void generateLegalMoves(Move(*moves)[], bool sideToMove, int* size)
{
    Move temp[256] = { 0 };
    int tempSize = 0;
    generatePseudoLegalMoves(&temp, sideToMove, &tempSize);

    int outIdx = 0;

    for (int i = 0; i < tempSize; ++i) {
        Move m = temp[i];
        if (m == 0) break;


        // Make the move
        /*
        // Snapshot current aggregate bitboards into debug vectors
        std::vector<U64> before = { wPawnBB, wKnightBB, wBishopBB, wRookBB, wQueenBB, wKingBB,
                                    bPawnBB, bKnightBB, bBishopBB, bRookBB, bQueenBB, bKingBB,
                                    allWhiteBB, allBlackBB, allPiecesBB };
                                    */


        makeMove(m);

        // After makeMove, turn is toggled, opponent to move.
        // Find king square of the side that just moved (sideToMove)
        int kingSq = -1;
        if (sideToMove == WHITE) {
            kingSq = count_trailing_zeros(wKingBB);
        }
        else {
            kingSq = count_trailing_zeros(bKingBB);
        }

        // Generate opponent pseudo-legal moves and see if any captures the king
        Move opp[256] = { 0 };
        int oppSize = 0;
        generatePseudoLegalMoves(&opp, !sideToMove, &oppSize);

        bool kingAttacked = false;
        for (int j = 0; j < oppSize; ++j) {
            Move om = opp[j];
            if (om == 0) break;
            int oto = (om >> 6) & 0x3F;
            if (oto == kingSq) { kingAttacked = true; break; }
        }

        if ((m & 0xF000) == FLAG_CASTLE_K)
        {
            // Check if the king is currently in check
            Move opp[256] = { 0 };
            int oppSize = 0;

            unmakeMove(m);

            generatePseudoLegalMoves(&opp, !sideToMove, &oppSize);

            makeMove(m);

            for (int j = 0; j < oppSize; ++j) {
                Move om = opp[j];
                if (om == 0) break;
                int oto = (om >> 6) & 0x3F;
                if (oto == kingSq) { kingAttacked = true; break; }
            }

            if (!turn)
            {
                for (int j = 0; j < oppSize; ++j) {
                    Move om = opp[j];
                    if (om == 0) break;
                    int oto = (om >> 6) & 0x3F;
                    U64 castleMask = 0x70;
                    U64 otoMask = 1ULL << oto;
                    if (otoMask & castleMask) { kingAttacked = true; break; }
                }
            }
            else
            {
                for (int j = 0; j < oppSize; ++j) {
                    Move om = opp[j];
                    if (om == 0) break;
                    int oto = (om >> 6) & 0x3F;
                    U64 castleMask = 0x7000000000000000;
                    U64 otoMask = 1ULL << oto;
                    if (otoMask & castleMask) { kingAttacked = true; break; }
                }
            }
        }
        else if ((m & 0xF000) == FLAG_CASTLE_Q)
        {
            // Check if the king is currently in check
            Move opp[256] = { 0 };
            int oppSize = 0;

            unmakeMove(m);

            generatePseudoLegalMoves(&opp, !sideToMove, &oppSize);

            makeMove(m);

            for (int j = 0; j < oppSize; ++j) {
                Move om = opp[j];
                if (om == 0) break;
                int oto = (om >> 6) & 0x3F;
                if (oto == kingSq) { kingAttacked = true; break; }
            }

            if (!turn)
            {
                for (int j = 0; j < oppSize; ++j) {
                    Move om = opp[j];
                    if (om == 0) break;
                    int oto = (om >> 6) & 0x3F;
                    U64 castleMask = 0x1C;
                    U64 otoMask = 1ULL << oto;
                    if (otoMask & castleMask) { kingAttacked = true; break; }
                }
            }
            else
            {
                for (int j = 0; j < oppSize; ++j) {
                    Move om = opp[j];
                    if (om == 0) break;
                    int oto = (om >> 6) & 0x3F;
                    U64 castleMask = 0x1C00000000000000;
                    U64 otoMask = 1ULL << oto;
                    if (otoMask & castleMask) { kingAttacked = true; break; }
                }
            }
        }

        // Unmake the move
        unmakeMove(m);

        /*
        // Snapshot after unmake
        std::vector<U64> after = { wPawnBB, wKnightBB, wBishopBB, wRookBB, wQueenBB, wKingBB,
                                   bPawnBB, bKnightBB, bBishopBB, bRookBB, bQueenBB, bKingBB,
                                   allWhiteBB, allBlackBB, allPiecesBB };

        // Compare before and after and print debug if mismatch
        bool mismatch = false;
        for (size_t k = 0; k < before.size(); ++k) {
            if (before[k] != after[k]) {
                mismatch = true; break;
            }
        }
        if (mismatch) {
            std::cout << "DEBUG: Board mismatch around move " << moveToUCI(m) << "\n" << std::flush;
            const char* names[] = { "wP","wN","wB","wR","wQ","wK","bP","bN","bB","bR","bQ","bK","allW","allB","all" };
            for (size_t k = 0; k < before.size(); ++k) {
                if (before[k] != after[k]) {
                    std::cout << "  " << names[k] << ": before=" << std::endl;
                    printBitboard(before[k]);
                    std::cout << " after=" << std::endl;
                    printBitboard(after[k]);
                    std::cout << "\n" << std::flush;
                }
            }
        }

        */


        if (!kingAttacked) {
            // keep move
            (*moves)[outIdx++] = m;
            if (outIdx >= 256) break;
        }
    }

    // terminate list and output size
    if (outIdx < 256) (*moves)[outIdx] = 0;
    *size = outIdx;
}