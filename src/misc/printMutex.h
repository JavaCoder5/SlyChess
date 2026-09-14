#pragma once
#include <iostream>
#include <mutex>

inline std::mutex printMutex;

void lockPrintMutex();

void unlockPrintMutex();
