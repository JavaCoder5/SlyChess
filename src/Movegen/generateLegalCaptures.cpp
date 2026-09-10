#include "generateLegalCaptures.h"
#include "isSquareAttacked.h"

void generateLegalCaptures(Move(*moves)[], bool sideToMove, int* size)
{
    Move temp[256] = { 0 };
    int tempSize = 0;
    generatePseudoLegalCaptures(&temp, sideToMove, &tempSize);

    int outIdx = 0;

    for (int i = 0; i < tempSize; ++i) {
        Move m = temp[i];
        if (m == 0) break;

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

        // Unmake the move
        unmakeMove(m);

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