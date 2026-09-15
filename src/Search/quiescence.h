#pragma once
#include <src/Constants/Constants.h>
#include <src/Movegen/generateLegalCaptures.h>
#include "sortLegalMoves.h"
#include <src/Evaluation/evaluate.h>
#include <src/Movegen/isSquareAttacked.h>

extern Move pvTable[MAX_DEPTH][MAX_DEPTH];
extern int pvLength[MAX_DEPTH];

extern int ply;

int quiescence(int alpha, int beta);