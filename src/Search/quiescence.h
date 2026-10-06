/*
    Copyright (C) 2026 JavaCoder5

    SlyChess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SlyChess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <src/Constants/Constants.h>
#include <src/Movegen/generateLegalCaptures.h>
#include "sortLegalMoves.h"
#include <src/Evaluation/evaluate.h>
#include <src/Movegen/isSquareAttacked.h>
#include <src/NNUE/nnue.h>

extern Move pvTable[MAX_DEPTH][MAX_DEPTH];
extern int pvLength[MAX_DEPTH];

extern int ply;

extern Accumulator wAccumulator, bAccumulator;

int quiescence(int alpha, int beta);