#pragma once
#include <src/Constants/Constants.h>

extern RepetitionEntry repetition_table[REPETITION_TABLE_SIZE];

void writeRepetitionTable(U64 boardHash, bool isOpen);

void clearRepetitionTableEntry(U64 boardHash);