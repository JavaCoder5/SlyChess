#include "moveToUci.h"

std::string moveToUCI(Move move)
{
	int fromSq = move & 0x3F; // bits 0-5
	int toSq = (move >> 6) & 0x3F; // bits 6-11
	int flags = move & 0xF000; // bits 12-15 (if needed for special move handling)
	char fromFile = 'a' + (fromSq % 8);
	char fromRank = '1' + (fromSq / 8);
	char toFile = 'a' + (toSq % 8);
	char toRank = '1' + (toSq / 8);
	char promotionSuffix;
	switch (flags) {
	case FLAG_PROMOTION_Q: promotionSuffix = 'q'; break;
	case FLAG_PROMOTION_R: promotionSuffix = 'r'; break;
	case FLAG_PROMOTION_B: promotionSuffix = 'b'; break;
	case FLAG_PROMOTION_N: promotionSuffix = 'n'; break;
	default: promotionSuffix = '\0'; break; // No promotion
	}

	return std::string() + fromFile + fromRank + toFile + toRank + promotionSuffix;
}