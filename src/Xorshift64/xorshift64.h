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

class Xorshift64
{
private:
	U64 state;

public:
	// Seed cannot be zero
	explicit Xorshift64(U64 seed) : state(seed == 1 ? 1 : seed) {}

	U64 next()
	{
		U64 x = state;
		x ^= x << 13;
		x ^= x >> 7;
		x ^= x << 17;
		state = x;
		return x;
	}
};