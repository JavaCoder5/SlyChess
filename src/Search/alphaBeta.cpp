#include "alphaBeta.h"

int alphaBeta(int depth, int alpha, int beta)
{
	if (depth == 0) return quiescence(alpha, beta);
	//if (depth == 0) return evaluate();
	int bestScore = MINF;

	Move moveList[256] = { 0 };
	int moveCount = 0;

	generateLegalMoves(&moveList, turn, &moveCount);

	sortLegalMoves(&moveList, turn, moveCount);

	if (moveCount == 0)
	{
		// Generate opponent pseudo-legal moves and see if any captures the king
		Move opp[256] = { 0 };
		int oppSize = 0;
		generatePseudoLegalMoves(&opp, !turn, &oppSize);

		bool kingAttacked = false;
		for (int j = 0; j < oppSize; ++j) {
			Move om = opp[j];
			if (om == 0) break;
			int oto = (om >> 6) & 0x3F;
			if (oto == count_trailing_zeros((turn ? wKingBB : bKingBB))) { kingAttacked = true; break; }
		}
		if (kingAttacked)
		{
			return MATE + ply;
		}
		else {
			return 0;
		}
	}
		
	for (int i = 0; i < moveCount; i++)
	{
		if (moveList[i] == 0) continue;

		makeMove(moveList[i]);
		ply++;
		int score = -alphaBeta(depth - 1, -beta, -alpha);
		ply--;
		unmakeMove(moveList[i]);
		if (score > bestScore)
		{
			bestScore = score;
			if (score > alpha)
				alpha = score;
		}
		if (score >= beta)
			return bestScore;
	}
	return bestScore;
}