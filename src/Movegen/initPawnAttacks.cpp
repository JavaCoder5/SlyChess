#include "initPawnAttacks.h"
#include <src/Constants/Constants.h>

void initWPawnAttacks(U64 (*wPawnAttacks)[64])
{
	const int rankOffsets[1] = { 1 };
	const int fileOffsets[1] = { 0 };

	for (int sq = 8; sq < 64; ++sq) {
		U64 bb = 0ULL;
		int rank = sq / 8;
		int file = sq % 8;

		for (int k = 0; k < 1; ++k) {
			int r2 = rank + rankOffsets[k];
			int f2 = file + fileOffsets[k];
			if (r2 >= 0 && r2 < 8 && f2 >= 0 && f2 < 8) {
				int sq2 = r2 * 8 + f2;
				bb |= 1ULL << sq2;
				if (rank == 1) { // Pawns on the second rank can attack two squares forward
					bb |= 1ULL << (sq2 + 8);
				}
			}
			
		}

		(*wPawnAttacks)[sq] = bb;
	}
	return;
}

void initBPawnAttacks(U64 (*bPawnAttacks)[64])
{
	const int rankOffsets[1] = {-1 };
	const int fileOffsets[1] = { 0 };

	for (int sq = 8; sq < 64; ++sq) {
		U64 bb = 0ULL;
		int rank = sq / 8;
		int file = sq % 8;

		for (int k = 0; k < 1; ++k) {
			int r2 = rank + rankOffsets[k];
			int f2 = file + fileOffsets[k];
			if (r2 >= 0 && r2 < 8 && f2 >= 0 && f2 < 8) {
				int sq2 = r2 * 8 + f2;
				bb |= 1ULL << sq2;
				if (rank == 6) { // Pawns on the seventh rank can attack two squares forward
					bb |= 1ULL << (sq2 - 8);
				}
			}

		}

		(*bPawnAttacks)[sq] = bb;
	}
	return;
}