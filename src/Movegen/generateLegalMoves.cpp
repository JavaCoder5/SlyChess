#include "generateLegalMoves.h"
#include "isSquareAttacked.h"

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

        // Snapshot king square before making the move (needed for castling checks)
        int kingFrom = -1;
        if (sideToMove == WHITE) kingFrom = count_trailing_zeros(wKingBB);
        else kingFrom = count_trailing_zeros(bKingBB);

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

        // Fast attack test using bitboards instead of generating opponent moves
        bool kingAttacked = isSquareAttacked(kingSq, !sideToMove);

        if ((m & 0xF000) == FLAG_CASTLE_K)
        {
            // Verify castling legality: king must not be in check before, through, or after the move
            // kingFrom was captured above before makeMove
            unmakeMove(m);
            if (isSquareAttacked(kingFrom, !sideToMove)) kingAttacked = true;
            makeMove(m);
            int throughSq = sideToMove == WHITE ? 5 : 61; // f1 / f8
            int toSq = sideToMove == WHITE ? 6 : 62; // g1 / g8
            if (isSquareAttacked(throughSq, !sideToMove) || isSquareAttacked(toSq, !sideToMove)) kingAttacked = true;
        }
        else if ((m & 0xF000) == FLAG_CASTLE_Q)
        {
            // Verify castling legality: king must not be in check before, through, or after the move
            unmakeMove(m);
            if (isSquareAttacked(kingFrom, !sideToMove)) kingAttacked = true;
            makeMove(m);
            int throughSq = sideToMove == WHITE ? 3 : 59; // d1 / d8
            int toSq = sideToMove == WHITE ? 2 : 58; // c1 / c8
            if (isSquareAttacked(throughSq, !sideToMove) || isSquareAttacked(toSq, !sideToMove)) kingAttacked = true;
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