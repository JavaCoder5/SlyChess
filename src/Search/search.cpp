#include "search.h"
#include "sortLegalMoves.h"

void search(int depth)
{
	Move moveList[256] = { 0 };
	int moveCount = 0;

	Move currentBest = moveList[0];
	int currentBestScore = MINF;

	generateLegalMoves(&moveList, turn, &moveCount);

	sortLegalMoves(&moveList, turn, moveCount);

	if (moveCount == 0)
	{
		std::cout << "bestmove 0000" << std::endl << std::flush;
		return;
	}

	for (int i = 0; i < moveCount; i++)
	{
		if (moveList[i] == 0) continue;

		makeMove(moveList[i]);
		int score = -alphaBeta(depth, MINF, INF);
		unmakeMove(moveList[i]);

		if (score >= currentBestScore) {
			currentBestScore = score;
			currentBest = moveList[i];
		}
	}

	std::cout << "bestmove " << moveToUCI(currentBest) << std::endl << std::flush;
}