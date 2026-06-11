#pragma once
#include <src/Constants/Constants.h>
#include <src/Movegen/generateLegalMoves.h>
#include <src/uci/uciToMove.h>
#include <src/uci/moveToUci.h>

U64 perft(int depth);

void perftDivide(int depth);