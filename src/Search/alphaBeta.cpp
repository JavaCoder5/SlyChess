#include "alphaBeta.h"

int alphaBeta(int depth, int alpha, int beta)
{
	if (depth == 0) return evaluate();
	int bestScore = MINF;

	Move moveList[256] = { 0 };
	int moveCount = 0;

	generateLegalMoves(&moveList, turn, &moveCount);

	sortLegalMoves(&moveList, turn, moveCount);

	if (moveCount == 0)
	{
		return MINF;
	}
		
	for (int i = 0; i < moveCount; i++)
	{
		if (moveList[i] == 0) continue;

		makeMove(moveList[i]);
		int score = -alphaBeta(depth - 1, -alpha, -beta);
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