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
	if (subtract)
	{
		for (std::size_t i = 0; i < HIDDEN_SIZE; i += 16) {
			__m256i accumulatorValues = _mm256_load_si256(reinterpret_cast<const __m256i*>(accumulator + i));
			__m256i featureValues = _mm256_load_si256(reinterpret_cast<const __m256i*>(feature + i));
			_mm256_store_si256(reinterpret_cast<__m256i*>(accumulator + i), _mm256_sub_epi16(accumulatorValues, featureValues));
		}
	}
	else
	{
		for (std::size_t i = 0; i < HIDDEN_SIZE; i += 16) {
			__m256i accumulatorValues = _mm256_load_si256(reinterpret_cast<const __m256i*>(accumulator + i));
			__m256i featureValues = _mm256_load_si256(reinterpret_cast<const __m256i*>(feature + i));
			
			_mm256_store_si256(reinterpret_cast<__m256i*>(accumulator + i), _mm256_add_epi16(accumulatorValues, featureValues));
		}
	}
	
}

int64_t nnue_avx2_screlu_dot(const int16_t* values, const int16_t* weights) {
	const __m256i zero = _mm256_setzero_si256();
	const __m256i maxActivation = _mm256_set1_epi16(QA);
	// 64-bit lanes: 1024 squared activations times output weights can overflow int32.
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
		__m256i products = _mm256_add_epi32(_mm256_mullo_epi32(squareLow, weightLow32),
			_mm256_mullo_epi32(squareHigh, weightHigh32));
		sum = _mm256_add_epi64(sum, _mm256_cvtepi32_epi64(_mm256_castsi256_si128(products)));
		sum = _mm256_add_epi64(sum, _mm256_cvtepi32_epi64(_mm256_extracti128_si256(products, 1)));
	}

	alignas(32) int64_t lanes[4];
	_mm256_store_si256(reinterpret_cast<__m256i*>(lanes), sum);
	int64_t result = 0;
	for (int64_t lane : lanes) result += lane;
	return result;
}

// SCReLU dot product with the standard bullet trick: v = clamp(x, 0, QA), then
// madd(v * w, v) = v^2 * w summed in pairs. v * w fits in int16 because |w| <= 127,
// and each int32 lane sums 2 * 64 terms (< 2^31) over the 1024 inputs.
int64_t nnue_avx2_screlu_dot_fast(const int16_t* values, const int16_t* weights) {
	const __m256i zero = _mm256_setzero_si256();
	const __m256i maxActivation = _mm256_set1_epi16(QA);
	__m256i sum = _mm256_setzero_si256();

	for (std::size_t i = 0; i < HIDDEN_SIZE; i += 16) {
		__m256i v = _mm256_load_si256(reinterpret_cast<const __m256i*>(values + i));
		v = _mm256_min_epi16(_mm256_max_epi16(v, zero), maxActivation);
		__m256i w = _mm256_load_si256(reinterpret_cast<const __m256i*>(weights + i));
		sum = _mm256_add_epi32(sum, _mm256_madd_epi16(_mm256_mullo_epi16(v, w), v));
	}

	__m128i half = _mm_add_epi32(_mm256_castsi256_si128(sum), _mm256_extracti128_si256(sum, 1));
	alignas(16) int32_t lanes[4];
	_mm_store_si128(reinterpret_cast<__m128i*>(lanes), half);
	return static_cast<int64_t>(lanes[0]) + lanes[1] + lanes[2] + lanes[3];
}

template <int NA, int NS>
static void update_n(int16_t* out, const int16_t* in, const int16_t* const* adds, const int16_t* const* subs) {
	for (std::size_t i = 0; i < HIDDEN_SIZE; i += 16) {
		__m256i v = _mm256_load_si256(reinterpret_cast<const __m256i*>(in + i));
		for (int a = 0; a < NA; ++a)
			v = _mm256_add_epi16(v, _mm256_load_si256(reinterpret_cast<const __m256i*>(adds[a] + i)));
		for (int s = 0; s < NS; ++s)
			v = _mm256_sub_epi16(v, _mm256_load_si256(reinterpret_cast<const __m256i*>(subs[s] + i)));
		_mm256_store_si256(reinterpret_cast<__m256i*>(out + i), v);
	}
}

void nnue_avx2_update(int16_t* out, const int16_t* in, const int16_t* const* adds, int numAdds,
                      const int16_t* const* subs, int numSubs) {
	switch (numAdds * 3 + numSubs) {
	case 0: update_n<0, 0>(out, in, adds, subs); break;
	case 1: update_n<0, 1>(out, in, adds, subs); break;
	case 2: update_n<0, 2>(out, in, adds, subs); break;
	case 3: update_n<1, 0>(out, in, adds, subs); break;
	case 4: update_n<1, 1>(out, in, adds, subs); break;
	case 5: update_n<1, 2>(out, in, adds, subs); break;
	case 6: update_n<2, 0>(out, in, adds, subs); break;
	case 7: update_n<2, 1>(out, in, adds, subs); break;
	case 8: update_n<2, 2>(out, in, adds, subs); break;
	}
}