#include "quiescence.h"

int quiescence(int alpha, int beta)
{
	int bestScore = MINF;

	Move moveList[256] = { 0 };
	int moveCount = 0;

	generateLegalMoves(&moveList, turn, &moveCount);

	sortLegalMoves(&moveList, turn, moveCount);

	if (moveCount == 0)
	{
		return evaluate();
	}

	for (int i = 0; i < moveCount; i++)
	{
		if (moveList[i] == 0) continue;

		makeMove(moveList[i]);
		int score = -quiescence(-alpha, -beta);
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