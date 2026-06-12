#include "evaluate.h"

inline constexpr int openingPawnPST[64] =
{

};


int evaluate()
{
	int whiteScore = 0;
	int blackScore = 0;

	whiteScore += mask_popcount(wPawnBB) * 100;
	whiteScore += mask_popcount(wKnightBB) * 300;
	whiteScore += mask_popcount(wBishopBB) * 320;
	whiteScore += mask_popcount(wRookBB) * 500;
	whiteScore += mask_popcount(wQueenBB) * 900;
	whiteScore += mask_popcount(wKingBB) * 20000;

	blackScore += mask_popcount(bPawnBB) * 100;
	blackScore += mask_popcount(bKnightBB) * 300;
	blackScore += mask_popcount(bBishopBB) * 320;
	blackScore += mask_popcount(bRookBB) * 500;
	blackScore += mask_popcount(bQueenBB) * 900;
	blackScore += mask_popcount(bKingBB) * 20000;

	return whiteScore - blackScore;
}