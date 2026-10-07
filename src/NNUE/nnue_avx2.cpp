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

#include "nnue_avx2.h"
#include "nnue.h"
#include <immintrin.h>

static_assert(HIDDEN_SIZE % 16 == 0);

void nnue_avx2_apply_feature_delta(int16_t* accumulator, const int16_t* feature, bool subtract) {
	for (std::size_t i = 0; i < HIDDEN_SIZE; i += 16) {
		__m256i accumulatorValues = _mm256_load_si256(reinterpret_cast<const __m256i*>(accumulator + i));
		__m256i featureValues = _mm256_load_si256(reinterpret_cast<const __m256i*>(feature + i));
		__m256i result = subtract
			? _mm256_sub_epi16(accumulatorValues, featureValues)
			: _mm256_add_epi16(accumulatorValues, featureValues);
		_mm256_store_si256(reinterpret_cast<__m256i*>(accumulator + i), result);
	}
}

int32_t nnue_avx2_screlu_dot(const int16_t* values, const int16_t* weights) {
	const __m256i zero = _mm256_setzero_si256();
	const __m256i maxActivation = _mm256_set1_epi16(QA);
	__m256i sum = _mm256_setzero_si256();

	for (std::size_t i = 0; i < HIDDEN_SIZE; i += 16) {
		__m256i activations = _mm256_load_si256(reinterpret_cast<const __m256i*>(values + i));
		activations = _mm256_min_epi16(_mm256_max_epi16(activations, zero), maxActivation);

		__m256i outputWeights = _mm256_load_si256(reinterpret_cast<const __m256i*>(weights + i));
		__m128i activationLow = _mm256_castsi256_si128(activations);
		__m128i activationHigh = _mm256_extracti128_si256(activations, 1);
		__m128i weightLow = _mm256_castsi256_si128(outputWeights);
		__m128i weightHigh = _mm256_extracti128_si256(outputWeights, 1);

		__m256i activationLow32 = _mm256_cvtepi16_epi32(activationLow);
		__m256i activationHigh32 = _mm256_cvtepi16_epi32(activationHigh);
		__m256i weightLow32 = _mm256_cvtepi16_epi32(weightLow);
		__m256i weightHigh32 = _mm256_cvtepi16_epi32(weightHigh);

		__m256i squareLow = _mm256_mullo_epi32(activationLow32, activationLow32);
		__m256i squareHigh = _mm256_mullo_epi32(activationHigh32, activationHigh32);
		sum = _mm256_add_epi32(sum, _mm256_mullo_epi32(squareLow, weightLow32));
		sum = _mm256_add_epi32(sum, _mm256_mullo_epi32(squareHigh, weightHigh32));
	}

	alignas(32) int32_t lanes[8];
	_mm256_store_si256(reinterpret_cast<__m256i*>(lanes), sum);
	int32_t result = 0;
	for (int32_t lane : lanes) result += lane;
	return result;
}
