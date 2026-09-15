#include "quiescence.h"

int quiescence(int alpha, int beta)
{
    // Leaf PV length
    pvLength[ply] = ply;

    int standPat = evaluate();

    if (standPat >= beta)
        return beta;

	bool isInCheck = isSquareAttacked(count_trailing_zeros((turn ? wKingBB : bKingBB)), !turn);

    if (!isInCheck)
    {
        int delta = 650;
        int promoDelta = 1600;
        if ((turn ? (wPawnBB & 0xFF000000000000) : (bPawnBB & 0xFF00)))
            delta = promoDelta;

        if (standPat < alpha - delta)
            return alpha;
    }

    if (standPat > alpha)
        alpha = standPat;

    Move moveList[256] = { 0 };
    int moveCount = 0;

    generateLegalCaptures(&moveList, turn, &moveCount);

    if (moveCount == 0)
        return standPat; // Avoids redundant evaluate() call

    sortLegalMoves(&moveList, turn, moveCount);

    int bestScore = standPat; // Track the best score properly

    for (int i = 0; i < moveCount; i++)
    {
        if (moveList[i] == 0) continue;

        makeMove(moveList[i]);
        ++ply;
        int score = -quiescence(-beta, -alpha);
        --ply;
        unmakeMove(moveList[i]);

        if (score > bestScore)
        {
            bestScore = score;

            if (score > alpha)
            {
                alpha = score;
                
                // Only PV nodes update the PV table
                if (score < beta) {
                    pvTable[ply][ply] = moveList[i];

                    // Copy child PV
                    for (int j = ply + 1; j < pvLength[ply + 1]; j++)
                        pvTable[ply][j] = pvTable[ply + 1][j];

                    pvLength[ply] = pvLength[ply + 1];
                }
            }
        }

        if (score >= beta)
            return beta; // Fail-high beta cutoff
    }

    return bestScore;
}