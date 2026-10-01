/*
    Copyright (C) 2026 JavaCoder5

    SlyChess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SlyChess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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