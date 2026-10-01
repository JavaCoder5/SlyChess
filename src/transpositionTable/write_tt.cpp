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