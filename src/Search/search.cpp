#include "search.h"
#include "sortLegalMoves.h"

U64 abNodes = 0;

Move pvTable[MAX_DEPTH][MAX_DEPTH];
int pvLength[MAX_DEPTH];

static std::string getPV()
{
	std::string pv = "";

	for (int i = 0; i < pvLength[0]; i++)
	{
		pv += moveToUCI(pvTable[0][i]);
		pv += " ";
	}

	return pv;
}

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
		lockPrintMutex();
		std::cout << "bestmove 0000" << std::endl << std::flush;
		unlockPrintMutex();
		return;
	}

	for (int iterativeDepth = 1; iterativeDepth <= depth; iterativeDepth++)
	{
		if (stopSearch) break;

		pvLength[0] = 0;

		int alpha = MINF;
		int beta = INF;

		abNodes = 0;

		auto searchStart = std::chrono::high_resolution_clock::now();

		for (int i = 0; i < moveCount; i++)
		{
			if (moveList[i] == 0) continue;

			makeMove(moveList[i]);
			ply++;
			int score = -alphaBeta(iterativeDepth - 1, -beta, -alpha);
			ply--;
			unmakeMove(moveList[i]);

			if ((score == MINF) && stopSearch) break;

			if (score > alpha)
			{
				alpha = score;
				currentBestScore = score;
				currentBest = moveList[i];

				// Only PV nodes update the PV table
				pvTable[ply][ply] = moveList[i];

				// Copy child PV
				for (int j = ply + 1; j < pvLength[ply + 1]; j++)
					pvTable[ply][j] = pvTable[ply + 1][j];

				pvLength[ply] = pvLength[ply + 1];
			}
		}

		auto searchEnd = std::chrono::high_resolution_clock::now();

		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(searchEnd - searchStart).count();

		if (currentBestScore > 99950)
		{
			lockPrintMutex();
			std::cout << "info depth " << iterativeDepth <<
				" score mate " << (-MATE - currentBestScore) / 2 + 1 <<
				" nps " << (abNodes * 1000000000) / (duration > 0 ? duration : 1) <<
				" ttHits " << ttHits <<
				" ttMisses " << ttMisses <<
				" nodes " << abNodes <<
				" pv " << getPV() <<
				std::endl << std::flush;
			unlockPrintMutex();
		}
		else if (currentBestScore < -99950)
		{
			lockPrintMutex();
			std::cout << "info depth " << iterativeDepth <<
				" score mate " << (MATE - currentBestScore) / 2 - 1 <<
				" nps " << (abNodes * 1000000000) / (duration > 0 ? duration : 1) <<
				" nodes " << abNodes <<
				" ttHits " << ttHits <<
				" ttMisses " << ttMisses <<
				" pv " << getPV() <<
				std::endl << std::flush;
			unlockPrintMutex();
		}
		else
		{
			lockPrintMutex();
			std::cout << "info depth " << iterativeDepth <<
				" score cp " << currentBestScore <<
				" nps " << (abNodes * 1000000000) / (duration > 0 ? duration : 1) <<
				" ttHits " << ttHits <<
				" ttMisses " << ttMisses <<
				" nodes " << abNodes <<
				" pv " << getPV() <<
				std::endl << std::flush;
			unlockPrintMutex();
		}

		movePVToFront(&moveList, moveCount, currentBest);

	}

	lockPrintMutex();
	std::cout << "bestmove " << moveToUCI(currentBest) << std::endl << std::flush;
	unlockPrintMutex();

	searchRunning = false;
}