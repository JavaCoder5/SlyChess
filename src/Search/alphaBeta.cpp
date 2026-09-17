#include "alphaBeta.h"

int alphaBeta(int depth, int alpha, int beta)
{
	abNodes++;

	if (abNodes & 8192 && stopSearch)
		return 0;

	if (depth == 0) return quiescence(alpha, beta);
	//if (depth == 0) return evaluate();
	int bestScore = MINF;

	int ttScore = 0;
	U8 ttDepth = 0;
	U8 ttFlag = 0;
	Move ttMove = 0;

	if (probe_tt(boardHash, ttMove, ttScore, ttDepth, ttFlag))
	{
		++ttHits;
		if (ttDepth >= depth)
		{
			if (ttFlag == TT_EXACT)
			{
				return ttScore;
			}
			if (ttFlag == TT_ALPHA && ttScore <= alpha)
			{
				return ttScore;
			}
			if (ttFlag == TT_BETA && ttScore >= beta)
			{
				return ttScore;
			}
		}
	}
	else
	{
		++ttMisses;
	}

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

	Move bestMove = 0;
		
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
			bestMove = moveList[i];
			if (score > alpha)
			{
				alpha = score;

				if (score >= beta)
				{
					write_tt(boardHash, moveList[i], score, depth, TT_BETA);
					return bestScore;
				}

				write_tt(boardHash, moveList[i], score, depth, TT_ALPHA);
			}
		}
	}
	if (bestMove != 0)
	{
		write_tt(boardHash, bestMove, bestScore, depth, TT_EXACT);
	}
	return bestScore;
}