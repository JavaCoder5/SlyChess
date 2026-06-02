#include "initKingAttacks.h"

void initKingAttacks(unsigned long long (*kingAttacks)[64])
{
	const int rankOffsets[8] = { 1, 1, 1, 0, -1, -1, -1, 0 };
	const int fileOffsets[8] = { 1, 0, -1, -1, -1, 0, 1, 1 };

	for (int sq = 0; sq < 64; ++sq) {
		unsigned long long bb = 0ULL;
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

		(*kingAttacks)[sq] = bb;
	}

	return;
}