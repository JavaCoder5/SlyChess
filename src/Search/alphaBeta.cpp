#include "alphaBeta.h"

bool hasNullMoved;

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
	abNodes++;

	if (depth <= 0) return quiescence(alpha, beta);

	if (abNodes & 8192 && stopSearch)
		return INF;

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
			// Adjust mate scores for current ply
			int correctedScore = ttScore;
			if (correctedScore <= MATE + 1000) correctedScore = MATE + ttDepth + ply;
			if (correctedScore >= -MATE - 1000) correctedScore = -MATE - ttDepth - ply;

			if (ttFlag == TT_EXACT)
			{
				pvLength[ply] = ply;
				return correctedScore;
			}
			if (ttFlag == TT_ALPHA && correctedScore <= alpha)
			{
				pvLength[ply] = ply;
				return correctedScore;
			}
			if (ttFlag == TT_BETA && correctedScore >= beta)
			{
				pvLength[ply] = ply;
				return correctedScore;
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

	Move pvMove = pvTable[ply][ply];

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
		
	for (int i = 0; i < moveCount; i++)
	{
		if (moveList[i] == 0) continue;

		int score;

		int reduction = 0;

		// Null move pruning
		bool isPvNode = (beta - alpha > 1);

		bool isInCheck = isSquareAttacked(count_trailing_zeros((turn ? wKingBB : bKingBB)), !turn);
		
		if (depth >= 4 &&
			(turn ? (wKnightBB | wBishopBB | wRookBB | wQueenBB) : (bKnightBB | bBishopBB | bRookBB | bQueenBB)) &&
			!hasNullMoved &&
			!isPvNode &&
			!isInCheck)
		{
			int R = 3;

			makeNullMove();
			int nullScore = -alphaBeta(depth - 1 - R, -beta, -beta + 1);
			unmakeNullMove();

			if (nullScore >= beta)
			{
				pvLength[ply] = ply;
				return beta;
			}
		}

		makeMove(moveList[i]);
		ply++;

		isInCheck = isSquareAttacked(count_trailing_zeros((turn ? wKingBB : bKingBB)), !turn);

		if (depth >= 3
			&& i >= 3
			&& !wasCapture()
			&& !wasPromotion()
			&& !isInCheck)
		{
			//reduction = (i / 2.5) * (depth / 4);
			reduction = 1;
		}

		if (reduction > 0)
		{
			score = -alphaBeta(depth - 1 - reduction, -alpha - 5, -alpha);

			if (score > alpha)
				score = -alphaBeta(depth - 1, -beta, -alpha);
		}
		else
		{
			score = -alphaBeta(depth - 1, -beta, -alpha);
		}

		ply--;
		unmakeMove(moveList[i]);

		if ((score == MINF) && stopSearch)
			return INF;

		if (score > bestScore)
		{
			bestScore = score;
			bestMove = moveList[i];

			// Only PV nodes update the PV table
			if (score > alpha && score < beta) {
			pvTable[ply][ply] = moveList[i];

				// Copy child PV
				for (int j = ply + 1; j < pvLength[ply + 1]; j++)
					pvTable[ply][j] = pvTable[ply + 1][j];
			
				pvLength[ply] = pvLength[ply + 1];
			}

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
	write_tt(boardHash, bestMove, bestScore, depth, TT_EXACT);
	return bestScore;
}