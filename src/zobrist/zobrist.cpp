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
