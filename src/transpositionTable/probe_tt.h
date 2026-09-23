#pragma once
#include <src/Constants/Constants.h>

extern TTEntry transposition_table[];

// Probe the transposition table for an entry matching 'key'. If found, fills
// the out-parameters with the stored move, score, depth and flag and returns
// true. Returns false when there is no matching entry.
bool probe_tt(U64 key, Move &move, int &score, U8 &depth, U8 &flag);

