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