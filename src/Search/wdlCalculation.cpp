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

#include "wdlCalculation.h"
#include <cmath>
#include <algorithm>

WDLStats calculateWDL(int cpScore)
{
	// Convert centipawn score to logistic model for WDL calculation
	// This uses a 3-outcome model based on the standard approach used by
	// modern engines like Stockfish, calibrated to match game statistics

	// The model parameters are derived from extensive self-play statistics
	constexpr double EVAL_SCALE = 200.0;  // Scaling factor for evaluation
	constexpr double DRAW_SLOPE = 1.25;   // Controls how evaluation affects draw rate
	constexpr double DRAW_BASE = 0.5;     // Base draw rate at 0 evaluation

	// Normalize the score: convert to Elo-like units
	// Higher evaluation = better for the side to move
	double normalizedScore = static_cast<double>(cpScore) / EVAL_SCALE;

	// Clamp to prevent extreme values
	normalizedScore = std::clamp(normalizedScore, -10.0, 10.0);

	// Calculate draw rate using sigmoid-like function
	// Draw rate tends to decrease with larger evaluations
	double drawRate = DRAW_BASE / (1.0 + std::abs(normalizedScore) * DRAW_SLOPE);
	drawRate = std::clamp(drawRate, 0.0, 1.0);

	// Remaining probability split between wins and losses
	double nonDrawProb = 1.0 - drawRate;

	// Use logistic function to split win/loss probability
	// Positive scores = higher win probability
	double winRatio = 1.0 / (1.0 + std::pow(10.0, -normalizedScore));

	// Calculate probabilities
	double winProb = winRatio * nonDrawProb;
	double lossProb = (1.0 - winRatio) * nonDrawProb;

	// Clamp to valid range
	winProb = std::clamp(winProb, 0.0, 1.0);
	lossProb = std::clamp(lossProb, 0.0, 1.0);
	drawRate = std::clamp(drawRate, 0.0, 1.0);

	// Normalize to sum to 1000 as per UCI spec
	uint32_t wins = static_cast<uint32_t>(std::round(winProb * 1000));
	uint32_t draws = static_cast<uint32_t>(std::round(drawRate * 1000));
	uint32_t losses = static_cast<uint32_t>(std::round(lossProb * 1000));

	// Adjust for rounding to ensure sum = 1000
	uint32_t total = wins + draws + losses;
	if (total != 1000) {
		// Adjust draws to make the sum exactly 1000
		draws = 1000 - wins - losses;
	}

	return WDLStats{ wins, draws, losses };
}
