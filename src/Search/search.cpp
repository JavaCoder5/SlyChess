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

	for (int iterativeDepth = 1; iterativeDepth <= depth; iterativeDepth++)
	{
		int alpha = MINF;
		int beta = INF;

		for (int i = 0; i < moveCount; i++)
		{
			if (moveList[i] == 0) continue;

			makeMove(moveList[i]);
			ply++;
			int score = -alphaBeta(iterativeDepth - 1, -beta, -alpha);
			ply--;
			unmakeMove(moveList[i]);

			if (score > alpha)
			{
				alpha = score;
				currentBestScore = score;
				currentBest = moveList[i];
			}
		}

		if (currentBestScore > 99950)
		{
			std::cout << "info depth " << iterativeDepth <<
				" score mate " << (-MATE - currentBestScore) / 2 + 1 <<
				" pv " << moveToUCI(currentBest) <<
				std::endl << std::flush;
		}
		else if (currentBestScore < -99950)
		{
			std::cout << "info depth " << iterativeDepth <<
				" score mate " << (MATE - currentBestScore) / 2 - 1 <<
				" pv " << moveToUCI(currentBest) <<
				std::endl << std::flush;
		}
		else
		{
			std::cout << "info depth " << iterativeDepth <<
				" score cp " << currentBestScore <<
				" pv " << moveToUCI(currentBest) <<
				std::endl << std::flush;
		}

		movePVToFront(&moveList, moveCount, currentBest);

	}

	std::cout << "bestmove " << moveToUCI(currentBest) << std::endl << std::flush;
}