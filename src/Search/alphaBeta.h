#pragma once
#include <src/Constants/Constants.h>
#include <src/Movegen/generateLegalMoves.h>
#include <src/Evaluation/evaluate.h>
#include "sortLegalMoves.h"
#include "quiescence.h"
#include "movePVToFront.h"

extern int ply;

extern U64 abNodes;

extern std::atomic_bool stopSearch;
extern std::atomic_bool searchRunning;

extern Move pvTable[MAX_DEPTH][MAX_DEPTH];
extern int pvLength[MAX_DEPTH];

int alphaBeta(int depth, int alpha, int beta);