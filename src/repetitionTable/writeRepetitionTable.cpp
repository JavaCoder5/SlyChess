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

std::vector<U64> repetitionHistory;

void resetRepetitionHistory()
{
	repetitionHistory.clear();
	repetitionHistory.reserve(MAX_UNDO + 256);
}

void pushRepetitionPosition(U64 key)
{
	repetitionHistory.push_back(key);
}

void popRepetitionPosition()
{
	if (!repetitionHistory.empty())
		repetitionHistory.pop_back();
}

void ensureRepetitionHistory(U64 key)
{
	if (repetitionHistory.empty() || repetitionHistory.back() != key)
	{
		resetRepetitionHistory();
		pushRepetitionPosition(key);
	}
}
