#pragma once
#include <src/Constants/Constants.h>
#include <src/Movegen/generateLegalMoves.h>
#include <src/Evaluation/evaluate.h>
#include "sortLegalMoves.h"

int alphaBeta(int depth, int alpha, int beta);