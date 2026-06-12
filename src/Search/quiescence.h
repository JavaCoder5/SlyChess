#pragma once
#include <src/Constants/Constants.h>
#include <src/Movegen/generateLegalMoves.h>
#include "sortLegalMoves.h"
#include <src/Evaluation/evaluate.h>

int quiescence(int alpha, int beta);