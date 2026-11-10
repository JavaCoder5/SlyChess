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

#include "alphaBeta.h"

bool hasNullMoved;

extern int nullMoveReductionTable[MAX_DEPTH];

extern int lmrReductionTable[MAX_DEPTH][256];

extern int seeReductionTable[MAX_DEPTH][10];

// Futility pruning margin at frontier nodes (in centipawns)
// Static eval + margin must be >= alpha to avoid pruning quiet moves
constexpr int FUTILITY_MARGIN = 322;

struct NullMoveUndoInfo {
	int prevEPSquare;
};

NullMoveUndoInfo beforeNullMove;

void makeNullMove()
{
	turn = !turn;
	beforeNullMove.prevEPSquare = enPassantSquare;
	if (enPassantSquare != -1) {
		boardHash ^= Zobrist::enpassant[enPassantSquare & 7];
	}
	enPassantSquare = -1;
	hasNullMoved = true;
	boardHash ^= Zobrist::sideToMoveKey;
}

void unmakeNullMove()
{
	turn = !turn;
	enPassantSquare = beforeNullMove.prevEPSquare;
	if (enPassantSquare != -1) {
		boardHash ^= Zobrist::enpassant[enPassantSquare & 7];
	}
	hasNullMoved = false;
	boardHash ^= Zobrist::sideToMoveKey;
}

int alphaBeta(int depth, int alpha, int beta)
{
	++abNodes;
	Move pvMove = getPVMove(ply);
	resetPVNode(ply);

	int originalAlpha = alpha;

	if (depth <= 0) return quiescence(alpha, beta);

	if (abNodes & 8192 && stopSearch)
		return INF;

	if (ply >= MAX_PLY)
		return quiescence(alpha, beta);

	int bestScore = MINF;

	if (!hasNullMoved && (halfmoveClock >= 100 || isRepetition(boardHash, halfmoveClock)))
	{
		Move drawMoves[256] = { 0 };
		int drawMoveCount = 0;
		generateLegalMoves(&drawMoves, turn, &drawMoveCount);

		if (drawMoveCount == 0)
		{
			int kingSquare = count_trailing_zeros((turn ? wKingBB : bKingBB));
			return isSquareAttacked(kingSquare, !turn) ? MATE + ply : 0;
		}

		return 0;
	}

	bool isPvNode = (beta - alpha > 1);

	int ttScore = 0;
	U8 ttDepth = 0;
	U8 ttFlag = 0;
	Move ttMove = 0;

	if (probe_tt(boardHash, ttMove, ttScore, ttDepth, ttFlag))
	{
		++ttHits;
		if (ttDepth >= depth && !isPvNode && halfmoveClock + depth < 100)
		{
			// Adjust mate scores for current ply
			int correctedScore = ttScore;
			if (correctedScore <= MATE + 1000) correctedScore += ply;
			if (correctedScore >= -MATE - 1000) correctedScore -= ply;

			if (ttFlag == TT_EXACT)
			{
				return correctedScore;
			}
			if (ttFlag == TT_ALPHA && correctedScore <= alpha)
			{
				return correctedScore;
			}
			if (ttFlag == TT_BETA && correctedScore >= beta)
			{
				return correctedScore;
			}
		}
	}
	else
	{
		++ttMisses;
	}

	bool isInCheck = isSquareAttacked(count_trailing_zeros((turn ? wKingBB : bKingBB)), !turn);

	if (depth >= 4 &&
		halfmoveClock + depth < 100 &&
		(turn ? (wKnightBB | wBishopBB | wRookBB | wQueenBB) : (bKnightBB | bBishopBB | bRookBB | bQueenBB)) &&
		!hasNullMoved &&
		!isPvNode &&
		!isInCheck)
	{
		//int R = 3;

		makeNullMove();
		int nullScore = -alphaBeta(depth - 1 - 3, -beta, -beta + 1);
		unmakeNullMove();

		if (nullScore >= beta)
		{
			return beta;
		}
	}

	Move moveList[256] = { 0 };
	int moveCount = 0;
	
	generateLegalMoves(&moveList, turn, &moveCount);

	sortLegalMoves(&moveList, turn, moveCount);

	if (pvMove != 0)
	{
		movePVToFront(&moveList, moveCount, pvMove);
	}

	if (ttMove != 0 && ttMove != pvMove)
	{
		movePVToFront(&moveList, moveCount, ttMove);
	}

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
			pvLength[ply] = ply;
			return MATE + ply;
		}
		else {
			pvLength[ply] = ply;
			return 0;
		}
	}

	Move bestMove = 0;

	// Futility pruning preparation at frontier nodes (depth == 1)
	// Only apply to non-PV nodes when not in check
	int staticEval = 0;
	bool canUseFutilityPruning = false;
	if (depth == 1 && !isPvNode && !isInCheck)
	{
		staticEval = g_nnue_network->evaluate(wAccumulator, bAccumulator) * (turn ? 1 : -1);
		canUseFutilityPruning = true;
	}

	for (int i = 0; i < moveCount; i++)
	{
		if (moveList[i] == 0) continue;

		Move move = moveList[i];

		// Futility pruning at depth==1: skip quiet moves that cannot improve alpha
		// Never prune moves that give check - they must be searched
		if (canUseFutilityPruning)
		{
			// Extract move components
			int toSquare = (move >> 6) & 0x3F;
			U64 toSquareMask = 1ULL << toSquare;

			// Determine if move is a capture or promotion
			U64 opponentPieces = turn ? (bPawnBB | bKnightBB | bBishopBB | bRookBB | bQueenBB | bKingBB) 
									  : (wPawnBB | wKnightBB | wBishopBB | wRookBB | wQueenBB | wKingBB);
			bool isCapture = (toSquareMask & opponentPieces) != 0;

			// Check promotion flag (bits 12-15)
			int promotionFlag = (move >> 12) & 0xF;
			bool isPromotion = (promotionFlag >= 0x1 && promotionFlag <= 0x4);  // FLAG_PROMOTION_Q/R/B/N

			// Check if this move gives check to opponent
			bool movingIntoCheck = givesCheck(move);

			// Skip quiet moves at depth==1 if eval + margin cannot reach alpha
			// BUT never prune moves that give check - they're too tactically important
			if (!isCapture && 
				!isPromotion && 
				!movingIntoCheck && 
				(staticEval + FUTILITY_MARGIN < alpha) &&
				(i > 1))
			{
				continue;
			}
		}

		int score;

		int reduction = 0;


		int seeScore = staticExchangeEvaluation(move);

		if (seeScore < 0)
		{
			int seeIndex = -seeScore / 100;
			if (seeIndex > 9) seeIndex = 9;
			reduction += 1;
		}

		makeMove(move);
		ply++;
		pushRepetitionPosition(boardHash);

		isInCheck = isSquareAttacked(count_trailing_zeros((turn ? wKingBB : bKingBB)), !turn);

		if (depth >= 3
			&& i >= 3
			&& !wasCapture()
			&& !wasPromotion()
			&& !isInCheck)
		{
			//reduction = (i / 2.5) * (depth / 4);
			/*
			reduction = 1;
			if (i >= 7) reduction = 2;
			if (i >= 11) reduction = 3;
			if (i >= 15) reduction = 4;
			if (i >= 17) reduction = 5;
			*/
			reduction += 1;
		}

		// Principal Variation Search (PVS)
		// First move is searched with full window [alpha, beta]
		// Subsequent moves are searched with null-width window [-alpha-1, -alpha]
		// If the null-window scout search returns > alpha, we re-search with full window
		if (i == 0)
		{
			// First move: use full window search
			if (reduction > 0)
			{
				score = -alphaBeta(depth - 1 - reduction, -beta, -alpha);

				if (score > alpha)
					score = -alphaBeta(depth - 1, -beta, -alpha);
			}
			else
			{
				score = -alphaBeta(depth - 1, -beta, -alpha);
			}
		}
		else
		{
			// Subsequent moves: use null-width window for scout search
			if (reduction > 0)
			{
				score = -alphaBeta(depth - 1 - reduction, -alpha - 1, -alpha);

				if (score > alpha && score < beta)
					score = -alphaBeta(depth - 1, -beta, -alpha);
			}
			else
			{
				score = -alphaBeta(depth - 1, -alpha - 1, -alpha);

				if (score > alpha && score < beta)
					score = -alphaBeta(depth - 1, -beta, -alpha);
			}
		}

		popRepetitionPosition();
		ply--;
		unmakeMove(moveList[i]);

		if ((score == MINF) && stopSearch)
			return INF;

		if (score > bestScore)
		{
			bestScore = score;
			bestMove = moveList[i];

			if (score > alpha)
			{
				if (!hasNullMoved)
					updatePV(ply, moveList[i]);

				alpha = score;

				if (score >= beta)
				{
					if (score < MATE + 1000)
					{
						write_tt(boardHash, moveList[i], score - ply, depth, TT_BETA);
					}
					else if (score > -MATE - 1000)
					{
						write_tt(boardHash, moveList[i], score + ply, depth, TT_BETA);
					}
					else
					{
						write_tt(boardHash, moveList[i], score, depth, TT_BETA);
					}
					return bestScore;
				}
			}
		}
	}

	int TT_FLAG = (originalAlpha < alpha) ? TT_EXACT : TT_ALPHA;

	if (bestScore < MATE + 1000)
	{
		write_tt(boardHash, bestMove, bestScore - ply, depth, TT_FLAG);
	}
	else if (bestScore > -MATE - 1000)
	{
		write_tt(boardHash, bestMove, bestScore + ply, depth, TT_FLAG);
	}
	else
	{
		write_tt(boardHash, bestMove, bestScore, depth, TT_FLAG);
	}
	return bestScore;
}