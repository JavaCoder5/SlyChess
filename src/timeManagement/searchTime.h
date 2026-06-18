#pragma once
#include <thread>
#include <chrono>
#include <src/Search/stopAndJoinSearch.h>

extern bool turn;

void searchTime(int totalTime);

void searchTimeControl(int wTime, int bTime, int wInc, int bInc);