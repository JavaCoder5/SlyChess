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

#include "movePVToFront.h"

void movePVToFront(Move(*moves)[], int size, Move PV)
{
	// If the PV move is a null move, or the list is empty, do nothing
	if (PV == 0 || size < 1) return;

	int pvIndex = -1;

	// Find the index of the PV move
	for (int i = 0; i < size; i++)
	{
		if ((*moves)[i] == PV)
		{
			pvIndex = i;
			break;
		}
	}

	// Move PV move to front
	if (pvIndex > 0)
	{
		Move tempPV = (*moves)[pvIndex]; // Temporarily store the PV move

		// Shift everything before the PV move one index to the right

		for (int i = pvIndex; i > 0; i--)
		{
			(*moves)[i] = (*moves)[i - 1];

		}

		// Finally, move PV move to the front

		(*moves)[0] = PV;
	}
}