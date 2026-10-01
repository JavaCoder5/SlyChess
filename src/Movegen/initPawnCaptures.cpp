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

#include "initPawnCaptures.h"

void initWPawnCaptures(U64(*wPawnCaptures)[64])
{
	const int rankOffsets[2] = { 1, 1 };
	const int fileOffsets[2] = {-1, 1 };

	for (int sq = 8; sq < 64; ++sq) {
		U64 bb = 0ULL;
		int rank = sq / 8;
		int file = sq % 8;

		for (int k = 0; k < 2; ++k) {
			int r2 = rank + rankOffsets[k];
			int f2 = file + fileOffsets[k];
			if (r2 >= 0 && r2 < 8 && f2 >= 0 && f2 < 8) {
				int sq2 = r2 * 8 + f2;
				bb |= 1ULL << sq2;
			}

		}

		(*wPawnCaptures)[sq] = bb;
	}
	return;
}

void initBPawnCaptures(U64(*bPawnCaptures)[64])
{
	const int rankOffsets[2] = { -1,-1 };
	const int fileOffsets[2] = { -1, 1 };

	for (int sq = 8; sq < 64; ++sq) {
		U64 bb = 0ULL;
		int rank = sq / 8;
		int file = sq % 8;

		for (int k = 0; k < 2; ++k) {
			int r2 = rank + rankOffsets[k];
			int f2 = file + fileOffsets[k];
			if (r2 >= 0 && r2 < 8 && f2 >= 0 && f2 < 8) {
				int sq2 = r2 * 8 + f2;
				bb |= 1ULL << sq2;
			}

		}

		(*bPawnCaptures)[sq] = bb;
	}
	return;
}