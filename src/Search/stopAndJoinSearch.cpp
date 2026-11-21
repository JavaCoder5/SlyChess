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

#include "stopAndJoinSearch.h"
#include <src/timeManagement/searchTime.h>

// Only called from the UCI thread, which owns searchThread and timerThread.
void stopAndJoinSearch()
{
	stopSearch = true;

	if (searchThread.joinable())
		searchThread.join();

	// The timer may still be waiting (search stopped early) or may have just fired;
	// either way, join it before clearing the flag so it can't stop the next search.
	cancelSearchTimer();

	stopSearch = false;
}