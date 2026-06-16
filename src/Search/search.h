#pragma once
#include <src/Movegen/generateLegalMoves.h>
#include <src/Constants/Constants.h>
#include <src/Search/alphaBeta.h>
#include <src/uci/moveToUci.h>
#include "movePVToFront.h"

extern bool turn;

extern int ply;

void search(int depth);