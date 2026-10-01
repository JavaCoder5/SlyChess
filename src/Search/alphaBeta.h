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
#include <src/Movegen/generateLegalMoves.h>
#include <src/Evaluation/evaluate.h>
#include "sortLegalMoves.h"
#include "quiescence.h"
#include <src/transpositionTable/probe_tt.h>
#include <src/transpositionTable/write_tt.h>
#include "movePVToFront.h"
#include <src/Helpers/wasCapture.h>
#include <src/Helpers/wasPromotion.h>
#include <src/Movegen/isSquareAttacked.h>
#include <src/repetitionTable/isRepetition.h>
#include <src/repetitionTable/writeRepetitionTable.h>

extern int ply;

extern U64 abNodes;

extern std::atomic_bool stopSearch;
extern std::atomic_bool searchRunning;

extern U64 ttHits, ttMisses;

extern Move pvTable[MAX_DEPTH][MAX_DEPTH];
extern int pvLength[MAX_DEPTH];

extern U64 wPawnBB;
extern U64 wKnightBB;
extern U64 wBishopBB;
extern U64 wRookBB;
extern U64 wQueenBB;
extern U64 wKingBB;

extern U64 bPawnBB;
extern U64 bKnightBB;
extern U64 bBishopBB;
extern U64 bRookBB;
extern U64 bQueenBB;
extern U64 bKingBB;

int alphaBeta(int depth, int alpha, int beta);