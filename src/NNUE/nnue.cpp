#include "nnue.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <src/Constants/Macros.h>

std::unique_ptr<Network> g_nnue_network = nullptr;

void Accumulator::init(const Network& net) {
    std::memcpy(vals, net.feature_bias.vals, sizeof(vals));
}

void Accumulator::add_feature(std::size_t feature_idx, const Network& net) {
    for (std::size_t i = 0; i < HIDDEN_SIZE; ++i) {
        vals[i] += net.feature_weights[feature_idx].vals[i];
    }
}

void Accumulator::remove_feature(std::size_t feature_idx, const Network& net) {
    for (std::size_t i = 0; i < HIDDEN_SIZE; ++i) {
        vals[i] -= net.feature_weights[feature_idx].vals[i];
    }
}

inline int32_t screlu(int16_t x) {
    int32_t y = std::clamp(static_cast<int32_t>(x), 0, static_cast<int32_t>(QA));
    return y * y;
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
    int32_t output = 0;
    for (std::size_t i = 0; i < HIDDEN_SIZE; ++i) {
        output += screlu(us.vals[i]) * static_cast<int32_t>(output_weights[i]);
    }
    for (std::size_t i = 0; i < HIDDEN_SIZE; ++i) {
        output += screlu(them.vals[i]) * static_cast<int32_t>(output_weights[HIDDEN_SIZE + i]);
    }
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
    g_nnue_network = std::move(net);
    return true;
}