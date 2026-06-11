#pragma once
#include <src/Constants/Constants.h>
#include "generatePseudoLegalMoves.h"
#include <src/Movegen/Board/makeMove.h>
#include <src/Movegen/Board/unmakeMove.h>

void generateLegalMoves(Move(*moves)[], bool sideToMove, int* size);