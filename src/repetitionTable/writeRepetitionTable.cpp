#include "writeRepetitionTable.h"
#include <iostream>

void writeRepetitionTable(U64 boardHash, bool isOpen)
{
	int index = boardHash & REPETITION_TABLE_MASK;
	if (repetition_table[index].key == 0)
	{
		repetition_table[index].key = boardHash;
		repetition_table[index].isOpen = isOpen;
	}
	else if (repetition_table[index].key == boardHash)
	{
		repetition_table[index].isOpen = isOpen;
	}
	else
	{
		// Collision: Overwrite the existing entry
		repetition_table[index].key = boardHash;
		repetition_table[index].isOpen = isOpen;
	}

}

void clearRepetitionTableEntry(U64 boardHash)
{
	int index = boardHash & REPETITION_TABLE_MASK;
	repetition_table[index] = RepetitionEntry();
}