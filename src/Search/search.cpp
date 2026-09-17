#include "search.h"
#include "sortLegalMoves.h"

U64 abNodes = 0;

U64 ttHits = 0;
U64 ttMisses = 0;

void search(int depth)
{
	searchRunning = true;

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
		if (stopSearch) break;

		int alpha = MINF;
		int beta = INF;

		currentBest = moveList[0];
		currentBestScore = MINF;

		abNodes = 0;

		auto searchStart = std::chrono::high_resolution_clock::now();

		for (int i = 0; i < moveCount; i++)
		{
			if (moveList[i] == 0) continue;

			if (stopSearch) break;

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

		auto searchEnd = std::chrono::high_resolution_clock::now();

		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(searchEnd - searchStart).count();

		if (currentBestScore > 99950)
		{
			std::cout << "info depth " << iterativeDepth <<
				" score mate " << (-MATE - currentBestScore) / 2 + 1 <<
				" nps " << (abNodes * 1000000000) / (duration > 0 ? duration : 1) <<
				" ttHits " << ttHits <<
				" ttMisses " << ttMisses <<
				" pv " << moveToUCI(currentBest) <<
				std::endl << std::flush;
		}
		else if (currentBestScore < -99950)
		{
			std::cout << "info depth " << iterativeDepth <<
				" score mate " << (MATE - currentBestScore) / 2 - 1 <<
				" nps " << (abNodes * 1000000000) / (duration > 0 ? duration : 1) <<
				" ttHits " << ttHits <<
				" ttMisses " << ttMisses <<
				" pv " << moveToUCI(currentBest) <<
				std::endl << std::flush;
		}
		else
		{
			std::cout << "info depth " << iterativeDepth <<
				" score cp " << currentBestScore <<
				" nps " << (abNodes * 1000000000) / (duration > 0 ? duration : 1) <<
				" ttHits " << ttHits <<
				" ttMisses " << ttMisses <<
				" pv " << moveToUCI(currentBest) <<
				std::endl << std::flush;
		}

		movePVToFront(&moveList, moveCount, currentBest);

	}

	std::cout << "bestmove " << moveToUCI(currentBest) << std::endl << std::flush;

	searchRunning = false;
}