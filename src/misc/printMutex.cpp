#include "printMutex.h"

void lockPrintMutex() { printMutex.lock(); }

void unlockPrintMutex() { printMutex.unlock(); }