#include "nnue.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <src/Constants/Macros.h>
#if defined(SLYCHESS_HAS_AVX2)
#include "nnue_avx2.h"
#if defined(_MSC_VER)
#include <intrin.h>
#endif
#endif

std::unique_ptr<Network> g_nnue_network = nullptr;

namespace {
    void apply_feature_delta_scalar(int16_t* accumulator, const int16_t* feature, bool subtract) {
        for (std::size_t i = 0; i < HIDDEN_SIZE; ++i) {
            if (subtract) accumulator[i] -= feature[i];
            else accumulator[i] += feature[i];
        }
    }

    int32_t screlu_scalar(int16_t value) {
        int32_t activation = value;
        if (activation < 0) activation = 0;
        if (activation > QA) activation = QA;
        return activation * activation;
    }

    int32_t screlu_dot_scalar(const int16_t* values, const int16_t* weights) {
        int32_t output = 0;
        for (std::size_t i = 0; i < HIDDEN_SIZE; ++i)
            output += screlu_scalar(values[i]) * static_cast<int32_t>(weights[i]);
        return output;
    }

    using FeatureDeltaFn = void(*)(int16_t*, const int16_t*, bool);
    using ScreluDotFn = int32_t(*)(const int16_t*, const int16_t*);
    FeatureDeltaFn featureDelta = apply_feature_delta_scalar;
    ScreluDotFn screluDot = screlu_dot_scalar;

#if defined(SLYCHESS_HAS_AVX2)
    bool cpu_supports_avx2() {
#if defined(_MSC_VER)
        int cpuInfo[4];
        __cpuid(cpuInfo, 1);
        constexpr int osxsave = 1 << 27;
        constexpr int avx = 1 << 28;
        if ((cpuInfo[2] & (osxsave | avx)) != (osxsave | avx)) return false;
        if ((_xgetbv(0) & 0x6) != 0x6) return false;
        __cpuidex(cpuInfo, 7, 0);
        return (cpuInfo[1] & (1 << 5)) != 0;
#else
        return __builtin_cpu_supports("avx2");
#endif
    }

    void configure_simd() {
        if (cpu_supports_avx2()) {
            featureDelta = nnue_avx2_apply_feature_delta;
            screluDot = nnue_avx2_screlu_dot;
        }
    }
#endif
}

void Accumulator::init(const Network& net) {
    std::memcpy(vals, net.feature_bias.vals, sizeof(vals));
}

void Accumulator::add_feature(std::size_t feature_idx, const Network& net) {
    featureDelta(vals, net.feature_weights[feature_idx].vals, false);
}

void Accumulator::remove_feature(std::size_t feature_idx, const Network& net) {
    featureDelta(vals, net.feature_weights[feature_idx].vals, true);
}

/**
 * Computes the 0..768 feature index for Bullet's Chess768 input format.
 * Layout typically: (color * 384) + (piece_type * 64) + square
 */
inline static std::size_t get_chess768_index(int color, int piece_type, int square, bool flip_colors) {
    int effective_color = flip_colors ? 1 - color : color;
    return (effective_color * 384) + (piece_type * 64) + square;
}

/**
 * Performs a full accumulator refresh directly from bitboards.
 *
 * @param acc          The accumulator to populate.
 * @param net          The NNUE Network structure containing feature weights and bias.
 * @param piece_bbs    2D array of bitboards: piece_bbs[color][piece_type]
 * @param flip_board Optional: set to true if building for Black's perspective
 *                     (vertically mirroring squares via square ^ 56 if your network expects it).
 * 
 * @param flip_colors Optional: set to true if you want to swap the color of the pieces for the feature index.
 */
void refresh_accumulator_from_bitboards(
    Accumulator& acc,
    const Network& net,
    const uint64_t piece_bbs[2][6],
    bool flip_board,
    bool flip_colors
) {
    // 1. Initialize accumulator with the network feature bias
    acc.init(net);

    // 2. Loop through both colors and all 6 piece types
    for (int c = 0; c < 2; ++c) {
        for (int p = 0; p < 6; ++p) {
            uint64_t bb = piece_bbs[c][p];

            while (bb != 0) {
                int square = count_trailing_zeros(bb);
				bb &= bb - 1; // Clear the least significant bit

                // If your network/dual-perspective requires a flipped perspective for the opponent:
                int target_square = flip_board ? (square ^ 56) : square;

                // Compute feature index and add to accumulator
                std::size_t feature_idx = get_chess768_index(c, p, target_square, flip_colors);
                acc.add_feature(feature_idx, net);
            }
        }
    }
}

int32_t Network::evaluate(const Accumulator& us, const Accumulator& them) const {
    int32_t output = screluDot(us.vals, output_weights);
    output += screluDot(them.vals, output_weights + HIDDEN_SIZE);
    output = (output / QA) + output_bias;
    output *= SCALE;
    output /= (QA * QB);
    return output;
}

bool load_nnue(const std::string& filepath) {
    auto net = std::make_unique<Network>();
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Could not open NNUE file: " << filepath << "\n";
        return false;
    }
    file.read(reinterpret_cast<char*>(net.get()), sizeof(Network));
    if (!file) {
        std::cerr << "Error: Failed to read complete NNUE weights from " << filepath << "\n";
        return false;
    }
#if defined(SLYCHESS_HAS_AVX2)
    configure_simd();
#endif
    g_nnue_network = std::move(net);
    return true;
}