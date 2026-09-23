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