#include "probe_tt.h"

bool probe_tt(U64 key, Move &move, int &score, U8 &depth, U8 &flag)
{
	int index = key & TT_MASK;

	if (transposition_table[index].key == key) {
		move = transposition_table[index].move;
		score = transposition_table[index].score;
		depth = transposition_table[index].depth;
		flag = transposition_table[index].flag;
		return true;
	}

	return false;
}
