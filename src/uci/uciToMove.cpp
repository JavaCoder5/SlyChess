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

	if ((turn ? bPawnBB : wPawnBB) & (1ULL << (toSq + (turn ? -8 : 8))))
		if (~(allPiecesBB) & (1ULL << (toSq)))
			if ((turn ? wPawnBB : bPawnBB) & (1ULL << fromSq))
				return (fromSq) | (toSq << 6) | FLAG_EN_PASSANT;

	return (fromSq) | (toSq << 6);
}