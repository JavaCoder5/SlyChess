#include "initKnightAttacks.h"
#include <src/Constants/Constants.h>

// Function to initialize the knightAttacks array with attack bitboards on startup
void initKnightAttacks(U64 (*knightAttacks)[64])
{
	const int rankOffsets[8] = { 2, 1, -1, -2, -2, -1, 1, 2 };
	const int fileOffsets[8] = { 1, 2, 2, 1, -1, -2, -2, -1 };

	for (int sq = 0; sq < 64; ++sq) {
		U64 bb = 0ULL;
		int rank = sq / 8;
		int file = sq % 8;

		for (int k = 0; k < 8; ++k) {
			int r2 = rank + rankOffsets[k];
			int f2 = file + fileOffsets[k];
			if (r2 >= 0 && r2 < 8 && f2 >= 0 && f2 < 8) {
				int sq2 = r2 * 8 + f2;
				bb |= 1ULL << sq2;
			}
		}

		(*knightAttacks)[sq] = bb;
	}

	return;
}