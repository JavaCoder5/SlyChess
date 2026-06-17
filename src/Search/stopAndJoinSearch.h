#pragma once
#include <atomic>
#include <thread>

extern std::atomic_bool stopSearch;
extern std::atomic_bool searchRunning;

extern std::thread searchThread;

void stopAndJoinSearch();