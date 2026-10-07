#pragma once
#include <cstdint>

void nnue_avx2_apply_feature_delta(int16_t* accumulator, const int16_t* feature, bool subtract);
int32_t nnue_avx2_screlu_dot(const int16_t* values, const int16_t* weights);
