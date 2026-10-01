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

#pragma once
#include <src/Constants/Constants.h>

extern TTEntry transposition_table[];

// Probe the transposition table for an entry matching 'key'. If found, fills
// the out-parameters with the stored move, score, depth and flag and returns
// true. Returns false when there is no matching entry.
bool probe_tt(U64 key, Move &move, int &score, U8 &depth, U8 &flag);

