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

#include "zobrist.h"

namespace Zobrist {

	U64 psq[12][64];
	U64 castling[16];
	U64 enpassant[8];
	U64 sideToMoveKey;

	void init()
	{

		// pseudo rng
		Xorshift64 rng(0xCAFEBABE12345678ULL);

		// castling
		for (int i = 0; i < 16; ++i) {
			castling[i] = rng.next();
		}

		// en passant
		for (int file = 0; file < 8; file++) {
			enpassant[file] = rng.next();
		}

		// psq
		for (int p = 0; p < 12; ++p) {
			for (int sq = 0; sq < 64; ++sq) {
				psq[p][sq] = rng.next();
			}
		}

		// side key
		sideToMoveKey = rng.next();
	}
}
