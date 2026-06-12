#pragma once
#include "generatePseudoLegalCaptures.h"
#include <src/Movegen/Board/makeMove.h>
#include <src/Movegen/Board/unmakeMove.h>
#include <src/Movegen/generatePseudoLegalMoves.h>

void generateLegalCaptures(Move(*moves)[], bool sideToMove, int* size);