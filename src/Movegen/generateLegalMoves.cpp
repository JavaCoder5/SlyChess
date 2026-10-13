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

#include "generateLegalMoves.h"

static bool isSquareAttackedWithOccupancy(int sq, bool byWhite, U64 occupancy, U64 removedAttackers)
{
    if (sq < 0 || sq >= 64) return false;

    U64 sqBB = 1ULL << sq;
    U64 pawns = (byWhite ? wPawnBB : bPawnBB) & ~removedAttackers;
    int p1 = sq + (byWhite ? -7 : 7);
    int p2 = sq + (byWhite ? -9 : 9);
    if (p1 >= 0 && p1 < 64 && (pawns & (1ULL << p1)) &&
        ((byWhite ? wPawnCaptures : bPawnCaptures)[p1] & sqBB)) return true;
    if (p2 >= 0 && p2 < 64 && (pawns & (1ULL << p2)) &&
        ((byWhite ? wPawnCaptures : bPawnCaptures)[p2] & sqBB)) return true;

    U64 knights = (byWhite ? wKnightBB : bKnightBB) & ~removedAttackers;
    if (knightAttacks[sq] & knights) return true;

    U64 kings = (byWhite ? wKingBB : bKingBB) & ~removedAttackers;
    if (kingAttacks[sq] & kings) return true;

    U64 rooks = ((byWhite ? wRookBB : bRookBB) | (byWhite ? wQueenBB : bQueenBB)) & ~removedAttackers;
    if (lookupRookAttacks(sq, occupancy) & rooks) return true;

    U64 bishops = ((byWhite ? wBishopBB : bBishopBB) | (byWhite ? wQueenBB : bQueenBB)) & ~removedAttackers;
    return (lookupBishopAttacks(sq, occupancy) & bishops) != 0;
}

// Generate legal moves for sideToMove by filtering pseudo-legal moves.
void generateLegalMoves(Move(*moves)[], bool sideToMove, int* size)
{
    Move temp[256] = { 0 };
    int tempSize = 0;
    generatePseudoLegalMoves(&temp, sideToMove, &tempSize);

    const int kingSq = count_trailing_zeros(sideToMove ? wKingBB : bKingBB);
    const U64 kingBit = 1ULL << kingSq;
    const U64 opponentPieces = sideToMove ? allBlackBB : allWhiteBB;

    int outIdx = 0;

    for (int i = 0; i < tempSize; ++i) {
        Move m = temp[i];
        if (m == 0) break;

        int fromSq = m & 0x3F;
        int toSq = (m >> 6) & 0x3F;
        int flags = m & 0xF000;
        U64 fromBit = 1ULL << fromSq;
        U64 toBit = 1ULL << toSq;
        U64 capturedBit = opponentPieces & toBit;
        if (flags == FLAG_EN_PASSANT) {
            int capturedSq = sideToMove ? toSq - 8 : toSq + 8;
            capturedBit = 1ULL << capturedSq;
        }

        U64 occupancy = (allPiecesBB & ~fromBit & ~capturedBit) | toBit;
        bool kingAttacked;

        if (flags == FLAG_CASTLE_K || flags == FLAG_CASTLE_Q) {
            int throughSq = sideToMove
                ? (flags == FLAG_CASTLE_K ? 5 : 3)
                : (flags == FLAG_CASTLE_K ? 61 : 59);
            int rookFromSq = sideToMove
                ? (flags == FLAG_CASTLE_K ? 7 : 0)
                : (flags == FLAG_CASTLE_K ? 63 : 56);
            int rookToSq = sideToMove
                ? (flags == FLAG_CASTLE_K ? 5 : 3)
                : (flags == FLAG_CASTLE_K ? 61 : 59);
            U64 rookFromBit = 1ULL << rookFromSq;
            U64 ownRooks = sideToMove ? wRookBB : bRookBB;
            U64 throughOccupancy = (allPiecesBB & ~fromBit) | (1ULL << throughSq);
            U64 finalOccupancy = occupancy;
            if (ownRooks & rookFromBit) {
                finalOccupancy = (finalOccupancy & ~rookFromBit) | (1ULL << rookToSq);
            }

            kingAttacked = isSquareAttackedWithOccupancy(kingSq, !sideToMove, allPiecesBB, 0) ||
                isSquareAttackedWithOccupancy(throughSq, !sideToMove, throughOccupancy, 0) ||
                isSquareAttackedWithOccupancy(toSq, !sideToMove, finalOccupancy, 0);
        }
        else {
            int resultingKingSq = fromBit & kingBit ? toSq : kingSq;
            kingAttacked = isSquareAttackedWithOccupancy(resultingKingSq, !sideToMove, occupancy, capturedBit);
        }

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