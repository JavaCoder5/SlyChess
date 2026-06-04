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