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