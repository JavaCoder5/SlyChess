#pragma once
#include <src/Constants/Constants.h>
#include <src/Movegen/generateLegalMoves.h>
#include <src/Evaluation/evaluate.h>
#include "sortLegalMoves.h"
#include "quiescence.h"

extern int ply;

extern U64 abNodes;

int alphaBeta(int depth, int alpha, int beta);