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

#include "nnue.h"
#include <fstream>
#include <iostream>
#include <algorithm>
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

    // int64: with 1024 hidden neurons a full int32 sum of QA^2 * QB-scaled terms can overflow.
    int64_t screlu_dot_scalar(const int16_t* values, const int16_t* weights) {
        int64_t output = 0;
        for (std::size_t i = 0; i < HIDDEN_SIZE; ++i)
            output += static_cast<int64_t>(screlu_scalar(values[i])) * weights[i];
        return output;
    }

    using FeatureDeltaFn = void(*)(int16_t*, const int16_t*, bool);
    using ScreluDotFn = int64_t(*)(const int16_t*, const int16_t*);
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

// Port of feature_index() in slychess-nnue-trainer/main.rs. There the board comes
// from bullet's ChessBoard, which is already seen from the side to move (flipped
// vertically for black, "our" pieces as color 0), and `opponent` flips it once
// more for the other side. Here squares are absolute, so the flip is applied
// directly for the black perspective; the result is the same index.
std::size_t halfka_feature_index(bool perspective, bool pieceIsWhite, int pieceType,
                                 int square, int kingSquare) {
    const int orient = perspective ? 0 : 56;
    const int king = kingSquare ^ orient;
    const int kingFile = king % 8;
    const int kingRank = king / 8;
    const int mirror = kingFile < 4 ? 7 : 0;
    const int orientedSquare = square ^ orient ^ mirror;
    const int kingBucket = (7 - kingRank) * 4 + std::min(kingFile, 7 - kingFile);

    const int pieceColor = pieceIsWhite == perspective ? 0 : 1; // 0: the perspective's own
    const int pieceSlot = pieceType == 5 ? 10 : pieceType * 2 + pieceColor;

    return static_cast<std::size_t>(kingBucket) * (PIECE_SLOTS * 64)
         + static_cast<std::size_t>(pieceSlot) * 64 + orientedSquare;
}

void refresh_accumulator_from_bitboards(
    Accumulator& acc,
    const Network& net,
    const uint64_t piece_bbs[2][6],
    bool perspective
) {
    acc.init(net);
    const int kingSquare = count_trailing_zeros(piece_bbs[perspective ? 0 : 1][5]);

    for (int c = 0; c < 2; ++c) {
        for (int p = 0; p < 6; ++p) {
            uint64_t bb = piece_bbs[c][p];
            while (bb != 0) {
                int square = count_trailing_zeros(bb);
                bb &= bb - 1;
                acc.add_feature(halfka_feature_index(perspective, c == 0, p, square, kingSquare), net);
            }
        }
    }
}

int32_t Network::evaluate(const Accumulator& us, const Accumulator& them) const {
    int64_t output = screluDot(us.vals, output_weights);
    output += screluDot(them.vals, output_weights + HIDDEN_SIZE);
    output = (output / QA) + output_bias;
    output *= SCALE;
    output /= (QA * QB);
    return static_cast<int32_t>(output);
}

bool load_nnue(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Error: Could not open NNUE file: " << filepath << "\n";
        return false;
    }
    const auto fileSize = static_cast<std::size_t>(file.tellg());
    // The trainer pads quantised.bin to a multiple of 64 bytes, as does sizeof(Network).
    if (fileSize != sizeof(Network)) {
        std::cerr << "Error: " << filepath << " is " << fileSize << " bytes, but a HalfKAv2_hm "
                  << INPUT_SIZE << "->" << HIDDEN_SIZE << "x2->1 network is " << sizeof(Network)
                  << " bytes. Use quantised.bin from slychess-nnue-trainer.\n";
        return false;
    }
    file.seekg(0);
    auto net = std::make_unique<Network>();
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