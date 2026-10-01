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
#include <src/Xorshift64/xorshift64.h>

namespace Zobrist {

	// 12 pieces x 64 squares
	extern U64 psq[12][64];

	// 16 castling rights combos
	extern U64 castling[16];

	// 8 files for en passant
	extern U64 enpassant[8];

	// side-to-move key
	extern U64 sideToMoveKey;

	void init();
}