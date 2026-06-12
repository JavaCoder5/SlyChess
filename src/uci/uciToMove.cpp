#include "uciToMove.h"

Move UCIToMove(std::string move)
{
	if (move.length() > 5) return 0; // Invalid move string
	char fromFile = move[0];
	char fromRank = move[1];
	char toFile = move[2];
	char toRank = move[3];
	char promo = '\0';
	if (move.length() == 5) promo = move[4]; // Handle promotion suffix if present
	if (fromFile < 'a' || fromFile > 'h' || toFile < 'a' || toFile > 'h' ||
		fromRank < '1' || fromRank > '8' || toRank < '1' || toRank > '8') {
		return 0; // Invalid characters
	}
	int fromSq = (fromRank - '1') * 8 + (fromFile - 'a');
	int toSq = (toRank - '1') * 8 + (toFile - 'a');

	// Check for promotion suffix
	if (promo == 'q') return (fromSq) | (toSq << 6) | FLAG_PROMOTION_Q;
	else if (promo == 'r') return (fromSq) | (toSq << 6) | FLAG_PROMOTION_R;
	else if (promo == 'b') return (fromSq) | (toSq << 6) | FLAG_PROMOTION_B;
	else if (promo == 'n') return (fromSq) | (toSq << 6) | FLAG_PROMOTION_N;

	if ((1ULL << (toSq + (turn ? -8 : 8))))
	{
		std::cout << (1ULL << (toSq + (turn ? -8 : 8))) << std::endl;
	}
	else
	{
		std::cout << "0" << std::endl;
	}

	if ((turn ? bPawnBB : wPawnBB) & (1ULL << (toSq + (turn ? -8 : 8))))
		if (~(allPiecesBB) & (1ULL << (toSq)))
			return (fromSq) | (toSq << 6) | FLAG_EN_PASSANT;

	return (fromSq) | (toSq << 6);
}