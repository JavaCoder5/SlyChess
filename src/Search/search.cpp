/*
	Copyright (C) 2026 JavaCoder5

	SlyChess is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	SlyChess is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "search.h"
#include "sortLegalMoves.h"
#include <algorithm>


U64 abNodes = 0;
U64 qNodes = 0;

Move pvTable[MAX_PLY][MAX_PLY] = { 0 };
int pvLength[MAX_PLY + 1] = { 0 };

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
	ensureRepetitionHistory(boardHash);

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
		searchRunning = false;
		return;
	}

	if (halfmoveClock >= 100 || isRepetition(boardHash, halfmoveClock))
	{
		lockPrintMutex();
		std::cout << "info depth 0 score cp 0 nodes 0 pv" << std::endl;
		std::cout << "bestmove " << moveToUCI(moveList[0]) << std::endl << std::flush;
		unlockPrintMutex();
		searchRunning = false;
		return;
	}

	Move ttMove = 0;
	int ttScore = 0;
	U8 ttDepth = 0;
	U8 ttFlag = 0;

	if (probe_tt(boardHash, ttMove, ttScore, ttDepth, ttFlag))
	{
		if (ttMove != 0 && ttMove != moveList[0])
		{
			movePVToFront(&moveList, moveCount, ttMove);
		}
	}

	int searchDepth = std::clamp(depth, 1, MAX_DEPTH - 1);
	for (int iterativeDepth = 1; iterativeDepth <= searchDepth; iterativeDepth++)
	{
		if (stopSearch) break;

		resetPVNode(0);

		int alpha = MINF;
		int beta = INF;

		abNodes = 0;
		qNodes = 0;

		ttHits = 0;
		ttMisses = 0;

		auto searchStart = std::chrono::high_resolution_clock::now();

		for (int i = 0; i < moveCount; i++)
		{
			if (moveList[i] == 0) continue;

			makeMove(moveList[i]);
			ply++;
			pushRepetitionPosition(boardHash);
			int score = -alphaBeta(iterativeDepth - 1, -beta, -alpha);
			popRepetitionPosition();
			ply--;
			unmakeMove(moveList[i]);

			if ((score == MINF) && stopSearch) break;

			if (score > alpha)
			{
				alpha = score;
				currentBestScore = score;
				currentBest = moveList[i];

				updatePV(0, moveList[i]);
			}
		}

		auto searchEnd = std::chrono::high_resolution_clock::now();

		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(searchEnd - searchStart).count();

		U64 totalNodes = abNodes + qNodes;

		if (currentBestScore > 99950)
		{
			lockPrintMutex();
			std::cout << "info depth " << iterativeDepth <<
				" score mate " << (-MATE - currentBestScore) / 2 + 1 <<
				" nps " << (abNodes * 1000000000) / (duration > 0 ? duration : 1) <<
				" ttHits " << ttHits <<
				" ttMisses " << ttMisses <<
				" nodes " << totalNodes <<
				" qnodes " << qNodes <<
				" abnodes " << abNodes <<
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
				" nodes " << totalNodes <<
				" qnodes " << qNodes <<
				" abnodes " << abNodes <<
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
				" nodes " << totalNodes <<
				" qnodes " << qNodes <<
				" abnodes " << abNodes <<
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