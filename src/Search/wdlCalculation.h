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
#include <cstdint>

struct WDLStats {
	uint32_t wins;
	uint32_t draws;
	uint32_t losses;
};

/// Calculate Win/Draw/Loss probabilities from a centipawn evaluation score.
/// Uses logistic regression approximation to convert evaluation to probabilities.
/// The output is normalized so that wins + draws + losses = 1000.
/// @param cpScore The evaluation score in centipawns (positive = better for side to move)
/// @return WDLStats containing normalized win/draw/loss counts (sum = 1000)
WDLStats calculateWDL(int cpScore);
