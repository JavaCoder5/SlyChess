#pragma once
#include <src/Movegen/generateLegalMoves.h>
#include <src/Constants/Constants.h>
#include <src/Search/alphaBeta.h>
#include <src/uci/moveToUci.h>

extern bool turn;

void search(int depth);