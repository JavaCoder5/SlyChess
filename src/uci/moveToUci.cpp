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

	if (promotionSuffix != 0)
	{
		return std::string() + fromFile + fromRank + toFile + toRank + promotionSuffix;
	} 
	else
	{
		return std::string() + fromFile + fromRank + toFile + toRank;
	}
	
}