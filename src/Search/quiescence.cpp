#include "quiescence.h"

int quiescence(int alpha, int beta)
{
	int bestScore = MINF;

	int standPat = evaluate();

	// Leaf PV length
	pvLength[ply] = ply;

	if (standPat >= beta)
		return beta;
	if (standPat > alpha)
		alpha = standPat;

	Move moveList[256] = { 0 };
	int moveCount = 0;

	generateLegalCaptures(&moveList, turn, &moveCount);

	sortLegalMoves(&moveList, turn, moveCount);

	if (moveCount == 0)
	{
		return evaluate();
	}

	for (int i = 0; i < moveCount; i++)
	{
		if (moveList[i] == 0) continue;

		makeMove(moveList[i]);
		int score = -quiescence(-beta, -alpha);
		unmakeMove(moveList[i]);
		/*
		if (score > bestScore)
		{
			bestScore = score;
			if (score > alpha)
				alpha = score;
		}
		*/
		if (score > alpha)
			alpha = score;
		if (score >= beta)
			return beta;
	}

	return alpha;
}