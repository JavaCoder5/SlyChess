#include "write_tt.h"

void write_tt(U64 key, Move move, int score, U8 depth, U8 flag)
{
	int index = key & TT_MASK;

    // Replacement strategy: Only overwrite if the new data is deeper or equal depth
    if (transposition_table[index].key == 0 || depth >= transposition_table[index].depth) {
        transposition_table[index].key = key;
        transposition_table[index].move = move;
        transposition_table[index].score = score;
        transposition_table[index].depth = depth;
        transposition_table[index].flag = flag;
    }
}