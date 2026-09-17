#pragma once
#include <src/Constants/Constants.h>
#include <src/Movegen/generateLegalMoves.h>
#include <src/Evaluation/evaluate.h>
#include "sortLegalMoves.h"
#include "quiescence.h"
#include <src/transpositionTable/probe_tt.h>
#include <src/transpositionTable/write_tt.h>

extern int ply;

extern U64 abNodes;

extern std::atomic_bool stopSearch;
extern std::atomic_bool searchRunning;

extern U64 ttHits, ttMisses;

int alphaBeta(int depth, int alpha, int beta);