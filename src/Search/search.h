#pragma once
#include <src/Movegen/generateLegalMoves.h>
#include <src/Constants/Constants.h>
#include <src/Search/alphaBeta.h>
#include <src/uci/moveToUci.h>
#include "movePVToFront.h"
#include <chrono> 
#include <atomic>
#include <src/misc/printMutex.h>

extern bool turn;

extern int ply;

extern std::atomic_bool searchRunning;
extern std::atomic_bool stopSearch;

void search(int depth);