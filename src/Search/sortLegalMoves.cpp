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

#include "sortLegalMoves.h"
#include <src/Movegen/generatePseudoLegalMoves.h>

// pawn, knight, bishop, rook, queen, king
inline constexpr int PieceValue[6] = { 100, 300, 320, 500, 900, 20000 };

static inline int pieceAtSquare(int square, const U64 pieces[2][6])
{
    U64 bit = 1ULL << square;

    for (int piece = 0; piece < 6; ++piece)
        if ((pieces[0][piece] | pieces[1][piece]) & bit)
            return piece;

    return -1;
}

static inline U64 attackersTo(int square, U64 occupancy, const U64 pieces[2][6], int side)
{
    U64 attackers = (side == 0 ? bPawnCaptures[square] & pieces[0][0] : wPawnCaptures[square] & pieces[1][0]);
    attackers |= knightAttacks[square] & pieces[side][1];
    attackers |= kingAttacks[square] & pieces[side][5];
    attackers |= lookupBishopAttacks(square, occupancy) & (pieces[side][2] | pieces[side][4]);
    attackers |= lookupRookAttacks(square, occupancy) & (pieces[side][3] | pieces[side][4]);
    return attackers;
}

int staticExchangeEvaluation(const Move& m)
{
    const int fromSq = m & 0x3F;
    const int toSq = (m >> 6) & 0x3F;
    const int flags = m & 0xF000;
    if (fromSq == toSq) return 0;

    U64 pieces[2][6] = {
        { wPawnBB, wKnightBB, wBishopBB, wRookBB, wQueenBB, wKingBB },
        { bPawnBB, bKnightBB, bBishopBB, bRookBB, bQueenBB, bKingBB }
    };
    const int side = turn ? 0 : 1;
    const int opponent = side ^ 1;
    const U64 fromBit = 1ULL << fromSq;
    const U64 toBit = 1ULL << toSq;

    int movingPiece = -1;
    for (int piece = 0; piece < 6; ++piece) {
        if (pieces[side][piece] & fromBit) {
            movingPiece = piece;
            break;
        }
    }
    if (movingPiece < 0) return 0;

    // Find captured piece
    int capturedPiece = -1;
    for (int piece = 0; piece < 6; ++piece) {
        if (pieces[opponent][piece] & toBit) {
            capturedPiece = piece;
            break;
        }
    }

    // Handle promotions
    bool isPromotion = (flags >= 0xC000); // adjust based on your flag macros
    int promotedPiece = movingPiece;
    if (flags == FLAG_PROMOTION_Q) promotedPiece = 4;
    else if (flags == FLAG_PROMOTION_R) promotedPiece = 3;
    else if (flags == FLAG_PROMOTION_B) promotedPiece = 2;
    else if (flags == FLAG_PROMOTION_N) promotedPiece = 1;

    // If it's a quiet move and not a promotion, SEE is 0 (or not a capture)
    if (capturedPiece < 0 && !isPromotion) return 0;

    // Array to store the score at each step of the exchange
    int gains[32];
    int depth = 0;

    // Initial capture gain
    gains[0] = capturedPiece >= 0 ? PieceValue[capturedPiece] : 0;
    if (isPromotion) {
        gains[0] += PieceValue[promotedPiece] - PieceValue[0];
    }

    // Setup initial state for simulation
    U64 occupancy = allPiecesBB;
    occupancy &= ~fromBit;

    pieces[side][movingPiece] &= ~fromBit;
    if (capturedPiece >= 0) {
        pieces[opponent][capturedPiece] &= ~toBit;
    }
    pieces[side][promotedPiece] |= toBit;
    occupancy |= toBit;

    int pieceOnTarget = promotedPiece;
    int sideToCapture = opponent;

    // Build the exchange sequence
    while (true) {
        U64 attackers = attackersTo(toSq, occupancy, pieces, sideToCapture);
        int attackerPiece = -1;
        int attackerSquare = -1;

        // Find the least valuable piece of the attacking side
        for (int piece = 0; piece < 6; ++piece) {
            U64 candidates = attackers & pieces[sideToCapture][piece];
            if (candidates) {
                attackerPiece = piece;
                attackerSquare = count_trailing_zeros(candidates);
                break;
            }
        }

        if (attackerPiece < 0) break;

        // If the king is defending and it's the only attacker left, illegal/unsafe unless no other choice
        // (Simplified: standard SEE breaks if king captures and enemy has pieces left protecting)

        ++depth;
        gains[depth] = PieceValue[pieceOnTarget] - gains[depth - 1];

        // Do the capture in simulation boards
        const U64 attackerBit = 1ULL << attackerSquare;
        pieces[sideToCapture][attackerPiece] &= ~attackerBit;
        pieces[sideToCapture ^ 1][pieceOnTarget] &= ~toBit; // remove old piece from target square
        pieces[sideToCapture][attackerPiece] |= toBit;      // put attacker on target square
        occupancy &= ~attackerBit;

        pieceOnTarget = attackerPiece;
        sideToCapture ^= 1;

        // Stop early if a king captures
        if (attackerPiece == 5) break;
    }

    // Negamax backup loop (working backwards from the last depth)
    while (depth > 0)
    {
        gains[depth - 1] = -std::max(-gains[depth - 1], gains[depth]);
        --depth;
    }

    return gains[0];
}

void sortLegalMoves(Move(*moves)[], bool sideToMove, int moveCount)
{
	if (moveCount <= 1) return;

	auto get_attacker_value = [&](int fromSq) {
		U64 bit = 1ULL << fromSq;
		if (sideToMove == WHITE) {
			if (wPawnBB & bit) return 100;
			if (wKnightBB & bit) return 300;
			if (wBishopBB & bit) return 300;
			if (wRookBB & bit) return 500;
			if (wQueenBB & bit) return 900;
			if (wKingBB & bit) return 10000;
		}
		else {
			if (bPawnBB & bit) return 100;
			if (bKnightBB & bit) return 300;
			if (bBishopBB & bit) return 300;
			if (bRookBB & bit) return 500;
			if (bQueenBB & bit) return 900;
			if (bKingBB & bit) return 10000;
		}
		return 0;
	};

	auto get_victim_value = [&](Move m) {
		int toSq = (m >> 6) & 0x3F;
		int flags = m & 0xF000;

		// En-passant captures remove a pawn not on the to-square
		if (flags == FLAG_EN_PASSANT) {
			return 100; // pawn
		}

		U64 bit = 1ULL << toSq;
		if (sideToMove == WHITE) {
			// white captures black piece on to-square
			if (bPawnBB & bit) return 100;
			if (bKnightBB & bit) return 300;
			if (bBishopBB & bit) return 300;
			if (bRookBB & bit) return 500;
			if (bQueenBB & bit) return 900;
			if (bKingBB & bit) return 10000;
		}
		else {
			if (wPawnBB & bit) return 100;
			if (wKnightBB & bit) return 300;
			if (wBishopBB & bit) return 300;
			if (wRookBB & bit) return 500;
			if (wQueenBB & bit) return 900;
			if (wKingBB & bit) return 10000;
		}
		return 0;
	};

	// Compute scores for each move (higher = better). Captures get positive scores, others zero.
	int scores[256] = { 0 };
	for (int i = 0; i < moveCount && i < 256; ++i) {
		Move m = (*moves)[i];
		if (m == 0) { scores[i] = 0; continue; }
        int fromSq = m & 0x3F;
        int victim = get_victim_value(m);
        if (victim == 0) { scores[i] = 0; continue; }
        int attacker = get_attacker_value(fromSq);
        // MVV-LVA: larger victim value first, tiebreak by lower attacker (we subtract attacker)
        scores[i] = victim * 1000 - attacker;

        // SEE move ordering
        scores[i] += staticExchangeEvaluation(m) * 1000;
	}

	// Simple selection sort by descending score to keep captures first
	for (int i = 0; i < moveCount - 1; ++i) {
		int best = i;
		for (int j = i + 1; j < moveCount; ++j) {
			if (scores[j] > scores[best]) best = j;
		}
		if (best != i) {
			Move tmp = (*moves)[i]; (*moves)[i] = (*moves)[best]; (*moves)[best] = tmp;
			int ts = scores[i]; scores[i] = scores[best]; scores[best] = ts;
		}
	}
}