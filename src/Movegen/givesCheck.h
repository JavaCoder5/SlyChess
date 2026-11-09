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

#pragma once
#include <src/Constants/Constants.h>

// Forward declarations
bool isSquareAttacked(int sq, bool byWhite);

extern bool turn;
extern U64 wPawnBB, wKnightBB, wBishopBB, wRookBB, wQueenBB, wKingBB;
extern U64 bPawnBB, bKnightBB, bBishopBB, bRookBB, bQueenBB, bKingBB;

// Fast check to see if a move gives check to the opponent
// Returns true if moving from->to results in check on opponent's king
inline bool givesCheck(Move move)
{
	int fromSquare = move & 0x3F;
	int toSquare = (move >> 6) & 0x3F;
	int flags = (move >> 12) & 0xF;

	// Get opponent's king square
	int opponentKingSquare;
	bool movingPlayerIsWhite = turn;

	if (movingPlayerIsWhite) {
		// White is moving, so check black king
		opponentKingSquare = count_trailing_zeros(bKingBB);
	} else {
		// Black is moving, so check white king
		opponentKingSquare = count_trailing_zeros(wKingBB);
	}

	// Case 1: Direct check by the moving piece
	// The piece moves to 'toSquare', check if it attacks opponent king
	U64 toBit = 1ULL << toSquare;
	U64 fromBit = 1ULL << fromSquare;

	if (movingPlayerIsWhite) {
		// Determine which white piece is moving (after it moves to toSquare)
		if (wPawnBB & fromBit) {
			// White pawn moving to toSquare gives check if it attacks king
			// White pawns attack diagonally upward (increasing rank)
			if (toSquare + 7 == opponentKingSquare && (opponentKingSquare % 8) != 7) return true;
			if (toSquare + 9 == opponentKingSquare && (opponentKingSquare % 8) != 0) return true;
		}
		else if (wKnightBB & fromBit) {
			// Knight move attacks
			int diff = toSquare - opponentKingSquare;
			if (diff == 6 || diff == 10 || diff == 15 || diff == 17 || diff == -6 || diff == -10 || diff == -15 || diff == -17) {
				return true;
			}
		}
		else if (wKingBB & fromBit) {
			// King move attacks (adjacent square)
			int diff = toSquare - opponentKingSquare;
			if (diff >= -1 && diff <= 1 && toSquare % 8 != 0 && toSquare % 8 != 7) return true;
			if ((toSquare - 8 == opponentKingSquare) || (toSquare + 8 == opponentKingSquare)) return true;
		}
		else {
			// Rook, Bishop, or Queen - check if path to king is clear
			// Create a temporary occupancy without the moving piece
			U64 tempOccupancy = (wPawnBB | wKnightBB | wBishopBB | wRookBB | wQueenBB | wKingBB |
								bPawnBB | bKnightBB | bBishopBB | bRookBB | bQueenBB | bKingBB);
			tempOccupancy &= ~fromBit;  // Remove piece from source
			tempOccupancy |= toBit;     // Add piece to destination (for capture handling)

			// Check if this piece at toSquare attacks the king
			if (wBishopBB & fromBit || wQueenBB & fromBit) {
				// Bishop or Queen - check diagonal
				int rankDiff = (toSquare / 8) - (opponentKingSquare / 8);
				int fileDiff = (toSquare % 8) - (opponentKingSquare % 8);
				if (abs(rankDiff) == abs(fileDiff)) {
					// On diagonal - check if path is clear
					int dirRank = (rankDiff > 0) ? 1 : (rankDiff < 0) ? -1 : 0;
					int dirFile = (fileDiff > 0) ? 1 : (fileDiff < 0) ? -1 : 0;
					int curSq = toSquare + dirRank * 8 + dirFile;
					while (curSq != opponentKingSquare) {
						if (tempOccupancy & (1ULL << curSq)) return false;
						curSq += dirRank * 8 + dirFile;
					}
					return true;
				}
			}
			if (wRookBB & fromBit || wQueenBB & fromBit) {
				// Rook or Queen - check straight line
				int rankDiff = (toSquare / 8) - (opponentKingSquare / 8);
				int fileDiff = (toSquare % 8) - (opponentKingSquare % 8);
				if ((rankDiff == 0 && fileDiff != 0) || (fileDiff == 0 && rankDiff != 0)) {
					// On same rank or file
					int dirRank = (rankDiff > 0) ? 1 : (rankDiff < 0) ? -1 : 0;
					int dirFile = (fileDiff > 0) ? 1 : (fileDiff < 0) ? -1 : 0;
					int curSq = toSquare + dirRank * 8 + dirFile;
					while (curSq != opponentKingSquare) {
						if (tempOccupancy & (1ULL << curSq)) return false;
						curSq += dirRank * 8 + dirFile;
					}
					return true;
				}
			}
		}
	} else {
		// Black is moving, check white king
		if (bPawnBB & fromBit) {
			// Black pawn moving to toSquare gives check if it attacks king
			// Black pawns attack diagonally downward (decreasing rank)
			if (toSquare - 7 == opponentKingSquare && (opponentKingSquare % 8) != 0) return true;
			if (toSquare - 9 == opponentKingSquare && (opponentKingSquare % 8) != 7) return true;
		}
		else if (bKnightBB & fromBit) {
			// Knight move attacks
			int diff = toSquare - opponentKingSquare;
			if (diff == 6 || diff == 10 || diff == 15 || diff == 17 || diff == -6 || diff == -10 || diff == -15 || diff == -17) {
				return true;
			}
		}
		else if (bKingBB & fromBit) {
			// King move attacks
			int diff = toSquare - opponentKingSquare;
			if (diff >= -1 && diff <= 1 && toSquare % 8 != 0 && toSquare % 8 != 7) return true;
			if ((toSquare - 8 == opponentKingSquare) || (toSquare + 8 == opponentKingSquare)) return true;
		}
		else {
			// Rook, Bishop, or Queen
			U64 tempOccupancy = (wPawnBB | wKnightBB | wBishopBB | wRookBB | wQueenBB | wKingBB |
								bPawnBB | bKnightBB | bBishopBB | bRookBB | bQueenBB | bKingBB);
			tempOccupancy &= ~fromBit;
			tempOccupancy |= toBit;

			if (bBishopBB & fromBit || bQueenBB & fromBit) {
				int rankDiff = (toSquare / 8) - (opponentKingSquare / 8);
				int fileDiff = (toSquare % 8) - (opponentKingSquare % 8);
				if (abs(rankDiff) == abs(fileDiff)) {
					int dirRank = (rankDiff > 0) ? 1 : (rankDiff < 0) ? -1 : 0;
					int dirFile = (fileDiff > 0) ? 1 : (fileDiff < 0) ? -1 : 0;
					int curSq = toSquare + dirRank * 8 + dirFile;
					while (curSq != opponentKingSquare) {
						if (tempOccupancy & (1ULL << curSq)) return false;
						curSq += dirRank * 8 + dirFile;
					}
					return true;
				}
			}
			if (bRookBB & fromBit || bQueenBB & fromBit) {
				int rankDiff = (toSquare / 8) - (opponentKingSquare / 8);
				int fileDiff = (toSquare % 8) - (opponentKingSquare % 8);
				if ((rankDiff == 0 && fileDiff != 0) || (fileDiff == 0 && rankDiff != 0)) {
					int dirRank = (rankDiff > 0) ? 1 : (rankDiff < 0) ? -1 : 0;
					int dirFile = (fileDiff > 0) ? 1 : (fileDiff < 0) ? -1 : 0;
					int curSq = toSquare + dirRank * 8 + dirFile;
					while (curSq != opponentKingSquare) {
						if (tempOccupancy & (1ULL << curSq)) return false;
						curSq += dirRank * 8 + dirFile;
					}
					return true;
				}
			}
		}
	}

	return false;
}
