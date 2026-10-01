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