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

#include "searchTime.h"

void searchTime(int totalTime)
{
	int waitTime = totalTime;

	std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));

	stopAndJoinSearch();
	return;
}

void searchTimeControl(int wTime, int bTime, int wInc, int bInc)
{
	int waitTime = 0;

	if (turn)
	{
		// White to play (engine is playing as white)
		waitTime = wTime / 20 + wInc / 2;
	}
	else
	{
		// Black to play
		waitTime = bTime / 20 + bInc / 2;
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));

	stopAndJoinSearch();
	return;
}