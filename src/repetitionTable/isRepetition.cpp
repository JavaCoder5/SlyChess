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

#include "isRepetition.h"
#include "writeRepetitionTable.h"
#include <algorithm>

bool isRepetition(U64 key, int halfmoveClock)
{
    if (repetitionHistory.empty() || repetitionHistory.back() != key)
        return false;

    int current = static_cast<int>(repetitionHistory.size()) - 1;
    int first = current - std::min(halfmoveClock, current);
    int occurrences = 0;

    for (int index = current; index >= first; index -= 2)
    {
        if (repetitionHistory[index] == key && ++occurrences >= 3)
            return true;
    }

    return false;
}