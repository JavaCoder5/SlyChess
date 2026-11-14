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
#include <cstddef>
#include <memory>
#include <string>

// Network architecture, matching JavaCoder5/slychess-nnue-trainer (bullet):
// HalfKAv2_hm inputs -> 2x1024 SCReLU (dual perspective) -> 1 output.
//
// HalfKAv2_hm: features are relative to the perspective's own king. The board is
// seen from that side (flipped vertically for black), mirrored horizontally so the
// king is on files e-h, and the king square picks one of 32 buckets. Both kings
// share one piece slot, so each bucket has 11 x 64 features.
constexpr std::size_t KING_BUCKETS = 32;
constexpr std::size_t PIECE_SLOTS = 11;
constexpr std::size_t INPUT_SIZE = KING_BUCKETS * PIECE_SLOTS * 64; // 22528
constexpr std::size_t HIDDEN_SIZE = 1024;
constexpr int32_t SCALE = 400;
constexpr int16_t QA = 255;
constexpr int16_t QB = 64;

struct Network;

struct alignas(64) Accumulator {
    int16_t vals[HIDDEN_SIZE];
    void init(const Network& net);
    void add_feature(std::size_t feature_idx, const Network& net);
    void remove_feature(std::size_t feature_idx, const Network& net);
};

// Same layout as the trainer's quantised.bin: l0w, l0b, l1w, l1b (then padding).
struct Network {
    Accumulator feature_weights[INPUT_SIZE];
    Accumulator feature_bias;
    int16_t output_weights[2 * HIDDEN_SIZE];
    int16_t output_bias;

    int32_t evaluate(const Accumulator& us, const Accumulator& them) const;
};

// Global network declaration (or managed via a singleton/engine class)
extern std::unique_ptr<Network> g_nnue_network;

// Feature index for one piece, seen from `perspective` (true: white).
// pieceIsWhite/pieceType (0..5: P N B R Q K)/square are absolute (a1 = 0);
// kingSquare is the perspective's own king square.
std::size_t halfka_feature_index(bool perspective, bool pieceIsWhite, int pieceType,
                                 int square, int kingSquare);

// Rebuild `acc` for `perspective` (true: white) from piece_bbs[color][piece_type],
// color 0 = white.
void refresh_accumulator_from_bitboards(
    Accumulator& acc,
    const Network& net,
    const uint64_t piece_bbs[2][6],
    bool perspective
);

// Evaluation in centipawns from the side to move's point of view.
inline int32_t evaluate_side_to_move(const Network& net, bool whiteToMove,
                                     const Accumulator& white, const Accumulator& black) {
    return whiteToMove ? net.evaluate(white, black) : net.evaluate(black, white);
}

bool load_nnue(const std::string& filepath);