#include "generateLegalCaptures.h"

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