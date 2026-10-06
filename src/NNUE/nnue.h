#pragma once
#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>

constexpr std::size_t HIDDEN_SIZE = 256;
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

struct Network {
    Accumulator feature_weights[768];
    Accumulator feature_bias;
    int16_t output_weights[2 * HIDDEN_SIZE];
    int16_t output_bias;

    int32_t evaluate(const Accumulator& us, const Accumulator& them) const;
};

// Global network declaration (or managed via a singleton/engine class)
extern std::unique_ptr<Network> g_nnue_network;

void refresh_accumulator_from_bitboards(
    Accumulator& acc,
    const Network& net,
    const uint64_t piece_bbs[2][6],
    bool flip_board = false,
    bool flip_pieces = false
);


bool load_nnue(const std::string& filepath);