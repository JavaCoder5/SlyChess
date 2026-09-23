#pragma once
#include <src/Constants/Constants.h>

extern TTEntry transposition_table[];

void write_tt(U64 key, Move move, int score, U8 depth, U8 flag);