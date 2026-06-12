#include "sortLegalMoves.h"
#include <src/Movegen/generatePseudoLegalMoves.h>

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