#include "evaluate.h"

inline constexpr int sideToMoveBonus = 20;

inline constexpr U64 whiteDevelopmentMask = 0xFF;

inline constexpr U64 blackDevelopmentMask = 0xFF00000000000000;

inline constexpr U64 whiteKingKCastlingMask = 0x40;

inline constexpr U64 blackKingKCastlingMask = 0x4000000000000000;

inline constexpr U64 whiteRookKCastlingMask = 0x20;

inline constexpr U64 blackRookKCastlingMask = 0x2000000000000000;

constexpr int openingPawnPST[64] =
{
	 0,  0,  0,  0,  0,  0,  0,  0,
	90, 90, 90, 90, 90, 90, 90, 90,
	10, 10, 20, 80, 80, 20, 10, 10,
	 5,  5, 10, 65, 65, 10,  5,  5,
	 0,  0,  0, 50, 50,  0,  0,  0,
	 5, -5,-10, 30, 10,-20,-10,  5,
	10, 10, 10,-25,-25, 15, 15, 15,
	 0,  0,  0,  0,  0,  0,  0,  0
};

constexpr int knightPST[64] =
{
    -50,-40,-30,-30,-30,-30,-40,-50,
	-40,-20,  0,  0,  0,  0,-20,-40,
	-30,  0, 25, 25, 25, 25,  0,-30,
	-30,  5, 25, 40, 40, 25,  5,-30,
	-30,  0, 25, 40, 40, 25,  0,-30,
	-30,  5, 25, 25, 25, 25,  5,-30,
	-40,-20,  0,  5,  5,  0,-20,-40,
	-50,-40,-20,-30,-30,-20,-40,-50
};

constexpr int bishopPST[64] =
{
	-20,-10,-10,-10,-10,-10,-10,-20,
	-10,  0,  0,  0,  0,  0,  0,-10,
	-10,  0,  5, 10, 10,  5,  0,-10,
	-10,  5,  5, 10, 10,  5,  5,-10,
	-10,  0, 25, 10, 10, 25,  0,-10,
	-10, 10, 10,  5,  5, 10, 10,-10,
	-10,  5,  0,  0,  0,  0,  5,-10,
	-20,-10,-40,-10,-10,-40,-10,-20,
};

constexpr int openingKingPST[64] = 
{
	-30, -40, -40, -50, -50, -40, -40, -30,
	-30, -40, -40, -50, -50, -40, -40, -30,
	-30, -40, -40, -50, -50, -40, -40, -30,
	-30, -40, -40, -50, -50, -40, -40, -30,
	-20, -30, -30, -40, -40, -30, -30, -20,
	-10, -20, -20, -20, -20, -20, -20, -10,
	 20,  20,   0,   0,   0,   0,  20,  20,
	 20,  30,  10, -30,   0, -50,  40,  20
};

constexpr int endgameKingPST[64] =
{
	-50,-40,-30,-30,-30,-30,-40,-50,
	-40,-20,  0,  0,  0,  0,-20,-40,
	-30,  0, 10, 10, 10, 10,  0,-30,
	-30,  5, 10, 15, 15, 10,  5,-30,
	-30,  0, 10, 15, 15, 10,  0,-30,
	-30,  5, 10, 10, 10, 10,  5,-30,
	-40,-20,  0,  5,  5,  0,-20,-40,
	-50,-40,-20,-30,-30,-20,-40,-50
};

inline int invertPST(const int (*pst)[64], int index)
{
	return (*pst)[index ^ 56];
}

int evaluate()
{
	int whiteScore = 0;
	int blackScore = 0;

	/*

	int scoreBefore = 0;
	int scoreAfter = 0;

	*/

	// Weigh pawns with their PST score
	U64 wPawnBBCopy = wPawnBB;
	while (wPawnBBCopy)
	{
		int sq = count_trailing_zeros(wPawnBBCopy);

		whiteScore += 100 + openingPawnPST[sq ^ 56];

		U64 bitMask = (1ULL << sq);
		wPawnBBCopy = wPawnBBCopy & ~bitMask; // Clear the least significant bit
	}

	/*

	scoreAfter = whiteScore;

	std::cout << "pawn score: " << scoreAfter - scoreBefore << std::endl;

	scoreBefore = scoreAfter;
	*/

	// Knights
	U64 wKnightBBCopy = wKnightBB;
	while (wKnightBBCopy)
	{
		int sq = count_trailing_zeros(wKnightBBCopy);

		whiteScore += 300 + knightPST[sq ^ 56];

		U64 bitMask = (1ULL << sq);
		wKnightBBCopy = wKnightBBCopy & ~bitMask; // Clear the least significant bit
	}

	/*

	scoreAfter = whiteScore;

	std::cout << "knight score: " << scoreAfter - scoreBefore << std::endl;

	scoreBefore = scoreAfter;
	*/

	// Bishops
	U64 wBishopBBCopy = wBishopBB;
	while (wBishopBBCopy)
	{
		int sq = count_trailing_zeros(wBishopBBCopy);

		whiteScore += 320 + bishopPST[sq ^ 56];

		U64 bitMask = (1ULL << sq);
		wBishopBBCopy = wBishopBBCopy & ~bitMask; // Clear the least significant bit
	}
	/*

	scoreAfter = whiteScore;

	std::cout << "bishop score: " << scoreAfter - scoreBefore << std::endl;

	scoreBefore = scoreAfter;
	*/

	// King
	U64 wKingBBCopy = wKingBB;
	while (wKingBBCopy)
	{
		int sq = count_trailing_zeros(wKingBBCopy);

		whiteScore += 20000 + ((mask_popcount(allPiecesBB) > 8) ? openingKingPST[sq ^ 56] : endgameKingPST[sq ^ 56]);

		U64 bitMask = (1ULL << sq);
		wKingBBCopy = wKingBBCopy & ~bitMask; // Clear the least significant bit
	}
	/*

	scoreAfter = whiteScore;

	std::cout << "king score: " << scoreAfter - scoreBefore << std::endl;

	scoreBefore = scoreAfter;
	*/

	whiteScore += mask_popcount(wRookBB) * 500;
	/*

	scoreAfter = whiteScore;

	std::cout << "rook score: " << scoreAfter - scoreBefore << std::endl;

	scoreBefore = scoreAfter;
	*/

	whiteScore += mask_popcount(wQueenBB) * 900;
	/*

	scoreAfter = whiteScore;

	std::cout << "queen score: " << scoreAfter - scoreBefore << std::endl;

	scoreBefore = scoreAfter;

	scoreBefore = blackScore;
	*/

	U64 bPawnBBCopy = bPawnBB;
	while (bPawnBBCopy)
	{
		int sq = count_trailing_zeros(bPawnBBCopy);

		blackScore += 100 + openingPawnPST[sq];

		U64 bitMask = (1ULL << sq);
		bPawnBBCopy = bPawnBBCopy & ~bitMask; // Clear the least significant bit
	}
	/*

	scoreAfter = blackScore;

	std::cout << "pawn score: " << scoreAfter - scoreBefore << std::endl;

	scoreBefore = scoreAfter;

	*/

	U64 bKnightBBCopy = bKnightBB;
	while (bKnightBBCopy)
	{
		int sq = count_trailing_zeros(bKnightBBCopy);

		blackScore += 300 + knightPST[sq];

		U64 bitMask = (1ULL << sq);
		bKnightBBCopy = bKnightBBCopy & ~bitMask; // Clear the least significant bit
	}

	/*

	scoreAfter = blackScore;

	std::cout << "knight score: " << scoreAfter - scoreBefore << std::endl;

	scoreBefore = scoreAfter;

	*/

	U64 bBishopBBCopy = bBishopBB;
	while (bBishopBBCopy)
	{
		int sq = count_trailing_zeros(bBishopBBCopy);

		blackScore += 320 + bishopPST[sq];

		U64 bitMask = (1ULL << sq);
		bBishopBBCopy = bBishopBBCopy & ~bitMask; // Clear the least significant bit
	}
	/*

	scoreAfter = blackScore;

	std::cout << "bishop score: " << scoreAfter - scoreBefore << std::endl;

	scoreBefore = scoreAfter;
	*/

	U64 bKingBBCopy = bKingBB;
	while (bKingBBCopy)
	{
		int sq = count_trailing_zeros(bKingBBCopy);

		blackScore += 20000 + ((mask_popcount(allPiecesBB) > 8) ? openingKingPST[sq] : endgameKingPST[sq]);

		U64 bitMask = (1ULL << sq);
		bKingBBCopy = bKingBBCopy & ~bitMask; // Clear the least significant bit
	}
	/*

	scoreAfter = blackScore;

	std::cout << "king score: " << scoreAfter - scoreBefore << std::endl;

	scoreBefore = scoreAfter;
	*/

	blackScore += mask_popcount(bRookBB) * 500;

	/*

	scoreAfter = blackScore;

	std::cout << "rook score: " << scoreAfter - scoreBefore << std::endl;

	scoreBefore = scoreAfter;
	*/

	blackScore += mask_popcount(bQueenBB) * 900;
	/*

	scoreAfter = blackScore;

	std::cout << "queen score: " << scoreAfter - scoreBefore << std::endl;

	scoreBefore = scoreAfter;
	*/

	// if it is white's turn, add side to move bonus to white, otherwise add it to black
	(turn ? whiteScore : blackScore) += sideToMoveBonus;
	/*

	scoreBefore = whiteScore;

	

	std::cout << "development score: " << scoreAfter - scoreBefore << std::endl;
	*/

	// Development bonus
	U64 whiteDevelopment = (wKnightBB | wBishopBB) & ~whiteDevelopmentMask;
	whiteScore += mask_popcount(whiteDevelopment) * 10;

	U64 blackDevelopment = (bKnightBB | bBishopBB) & ~blackDevelopmentMask;
	blackScore += mask_popcount(blackDevelopment) * 10;

	// Castling bonus (for now only kingside castling)
	U64 whiteIsKingCastled = (wKingBB & whiteKingKCastlingMask);
	U64 whiteIsRookCastled = (wRookBB & whiteRookKCastlingMask);

	if (whiteIsKingCastled && whiteIsRookCastled)
	{
		whiteScore += 20;
	}
	else if (whiteIsKingCastled)
	{
		whiteScore += 10;
	}

	U64 blackIsKingCastled = (bKingBB & blackKingKCastlingMask);
	U64 blackIsRookCastled = (bRookBB & blackRookKCastlingMask);

	if (blackIsKingCastled && blackIsRookCastled)
	{
		blackScore += 20;
	}
	else if (blackIsKingCastled)
	{
		blackScore += 10;
	}

	//std::cout << whiteScore << " " << blackScore << std::endl;

	int eval = whiteScore - blackScore;

	return (turn ? eval : -eval);
}