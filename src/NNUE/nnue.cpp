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
#if defined(_MSC_VER) && !defined(__clang__)
#include <xmmintrin.h>
#endif
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

    // out = in + adds[0..NA) - subs[0..NS), in one pass (out may equal in).
    template <int NA, int NS>
    void update_scalar_n(int16_t* out, const int16_t* in, const int16_t* const* adds, const int16_t* const* subs) {
        for (std::size_t i = 0; i < HIDDEN_SIZE; ++i) {
            int16_t v = in[i];
            for (int a = 0; a < NA; ++a) v = static_cast<int16_t>(v + adds[a][i]);
            for (int s = 0; s < NS; ++s) v = static_cast<int16_t>(v - subs[s][i]);
            out[i] = v;
        }
    }

    void update_scalar(int16_t* out, const int16_t* in, const int16_t* const* adds, int numAdds,
                       const int16_t* const* subs, int numSubs) {
        switch (numAdds * 3 + numSubs) {
        case 0: if (out != in) std::memcpy(out, in, HIDDEN_SIZE * sizeof(int16_t)); break;
        case 1: update_scalar_n<0, 1>(out, in, adds, subs); break;
        case 2: update_scalar_n<0, 2>(out, in, adds, subs); break;
        case 3: update_scalar_n<1, 0>(out, in, adds, subs); break;
        case 4: update_scalar_n<1, 1>(out, in, adds, subs); break;
        case 5: update_scalar_n<1, 2>(out, in, adds, subs); break;
        case 6: update_scalar_n<2, 0>(out, in, adds, subs); break;
        case 7: update_scalar_n<2, 1>(out, in, adds, subs); break;
        case 8: update_scalar_n<2, 2>(out, in, adds, subs); break;
        }
    }

    int32_t screlu_scalar(int16_t value) {
        int32_t activation = value;
        if (activation < 0) activation = 0;
        if (activation > QA) activation = QA;
        return activation * activation;
    }

    // Exact for any output weights: int64, since 1024 terms of QA^2 * w can overflow int32.
    int64_t screlu_dot_scalar(const int16_t* values, const int16_t* weights) {
        int64_t output = 0;
        for (std::size_t i = 0; i < HIDDEN_SIZE; ++i)
            output += static_cast<int64_t>(screlu_scalar(values[i])) * weights[i];
        return output;
    }

    // Fast path for |output weight| <= 127 (bullet clips QB-quantised weights to that):
    // then v * w fits in int16 (255 * 127 = 32385), like _mm256_madd_epi16 in the AVX2
    // version, and a block of 64 terms (< 64 * 255^2 * 127) fits in int32.
    int64_t screlu_dot_scalar_fast(const int16_t* values, const int16_t* weights) {
        int64_t output = 0;
        for (std::size_t block = 0; block < HIDDEN_SIZE; block += 64) {
            int32_t sum = 0;
            for (std::size_t i = block; i < block + 64; ++i) {
                int16_t v = values[i] < 0 ? 0 : (values[i] > QA ? QA : values[i]);
                int16_t vw = static_cast<int16_t>(v * weights[i]);
                sum += static_cast<int32_t>(vw) * v;
            }
            output += sum;
        }
        return output;
    }

    using FeatureDeltaFn = void(*)(int16_t*, const int16_t*, bool);
    using UpdateFn = void(*)(int16_t*, const int16_t*, const int16_t* const*, int, const int16_t* const*, int);
    using ScreluDotFn = int64_t(*)(const int16_t*, const int16_t*);
    FeatureDeltaFn featureDelta = apply_feature_delta_scalar;
    UpdateFn updateAccumulator = update_scalar;
    ScreluDotFn screluDot = screlu_dot_scalar;

    bool output_weights_fit_int8(const Network& net) {
        for (int16_t w : net.output_weights)
            if (w < -127 || w > 127) return false;
        return true;
    }

    [[maybe_unused]] bool cpu_supports_avx2() {
#if !defined(SLYCHESS_HAS_AVX2)
        return false;
#elif defined(_MSC_VER)
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

    void configure_kernels(const Network& net) {
        const bool fastOutput = output_weights_fit_int8(net);
#if defined(SLYCHESS_HAS_AVX2)
        if (cpu_supports_avx2()) {
            featureDelta = nnue_avx2_apply_feature_delta;
            updateAccumulator = nnue_avx2_update;
            screluDot = fastOutput ? nnue_avx2_screlu_dot_fast : nnue_avx2_screlu_dot;
            return;
        }
#endif
        featureDelta = apply_feature_delta_scalar;
        updateAccumulator = update_scalar;
        screluDot = fastOutput ? screlu_dot_scalar_fast : screlu_dot_scalar;
    }
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

// ---------------------------------------------------------------------------
// Accumulator stack and refresh cache
// ---------------------------------------------------------------------------
namespace {
    inline void prefetch_row(const int16_t* row) {
        for (std::size_t offset = 0; offset < HIDDEN_SIZE; offset += 64 / sizeof(int16_t)) {
#if defined(_MSC_VER) && !defined(__clang__)
            _mm_prefetch(reinterpret_cast<const char*>(row + offset), _MM_HINT_T0);
#else
            __builtin_prefetch(row + offset);
#endif
        }
    }

    constexpr int STACK_SIZE = 8192 + 1; // MAX_UNDO moves on top of the root position

    struct StackEntry {
        Accumulator acc[2];        // [0]: white perspective, [1]: black
        bool computed[2];
        bool needsRefresh[2];      // this perspective's king moved: no incremental update
        uint8_t numAdds[2], numSubs[2];
        uint32_t adds[2][2], subs[2][2];
        int kingSquare[2];
    };

    StackEntry g_stack[STACK_SIZE];
    int g_top = 0;

    // Refresh cache: for each perspective and own-king square, the accumulator of the
    // last position refreshed there and its pieces. A refresh only applies the
    // difference. (King square fixes both the bucket and the mirroring, so every
    // feature index is the same for all positions sharing an entry.)
    struct CacheEntry {
        Accumulator acc;
        uint64_t bbs[2][6];
    };
    CacheEntry g_cache[2][64];

    void reset_cache(const Network& net) {
        for (auto& perspective : g_cache)
            for (CacheEntry& e : perspective) {
                e.acc.init(net);
                std::memset(e.bbs, 0, sizeof(e.bbs));
            }
    }

    // Apply lists of feature adds/subs to `acc` in place, up to 2 + 2 per pass.
    void apply_lists(Accumulator& acc, const Network& net, const uint32_t* addIdx, int numAdds,
                     const uint32_t* subIdx, int numSubs) {
        int a = 0, s = 0;
        while (a < numAdds || s < numSubs) {
            const int16_t* adds[2];
            const int16_t* subs[2];
            int na = 0, ns = 0;
            while (na < 2 && a < numAdds) adds[na++] = net.feature_weights[addIdx[a++]].vals;
            while (ns < 2 && s < numSubs) subs[ns++] = net.feature_weights[subIdx[s++]].vals;
            updateAccumulator(acc.vals, acc.vals, adds, na, subs, ns);
        }
    }

    void refresh_cached(Accumulator& out, bool perspective, const uint64_t piece_bbs[2][6], int kingSquare) {
        const Network& net = *g_nnue_network;
        CacheEntry& e = g_cache[perspective ? 0 : 1][kingSquare];
        uint32_t addIdx[32], subIdx[32];
        int numAdds = 0, numSubs = 0;
        for (int c = 0; c < 2; ++c) {
            for (int p = 0; p < 6; ++p) {
                uint64_t added = piece_bbs[c][p] & ~e.bbs[c][p];
                uint64_t removed = e.bbs[c][p] & ~piece_bbs[c][p];
                while (added) {
                    int sq = count_trailing_zeros(added);
                    added &= added - 1;
                    addIdx[numAdds++] = static_cast<uint32_t>(halfka_feature_index(perspective, c == 0, p, sq, kingSquare));
                }
                while (removed) {
                    int sq = count_trailing_zeros(removed);
                    removed &= removed - 1;
                    subIdx[numSubs++] = static_cast<uint32_t>(halfka_feature_index(perspective, c == 0, p, sq, kingSquare));
                }
                e.bbs[c][p] = piece_bbs[c][p];
            }
        }
        apply_lists(e.acc, net, addIdx, numAdds, subIdx, numSubs);
        std::memcpy(out.vals, e.acc.vals, sizeof(out.vals));
    }

    // Make the top entry's accumulator for `perspective` up to date.
    void materialize(int perspective, const uint64_t piece_bbs[2][6]) {
        StackEntry& top = g_stack[g_top];
        if (top.computed[perspective]) return;

        // Walk back to the last computed entry, unless a king move is in the way.
        int i = g_top;
        while (!g_stack[i].computed[perspective] && !g_stack[i].needsRefresh[perspective]) --i;
        if (!g_stack[i].computed[perspective]) {
            refresh_cached(top.acc[perspective], perspective == 0, piece_bbs, top.kingSquare[perspective]);
            top.computed[perspective] = true;
            return;
        }

        const Network& net = *g_nnue_network;
        for (int j = i + 1; j <= g_top; ++j) {
            StackEntry& e = g_stack[j];
            const int16_t* adds[2];
            const int16_t* subs[2];
            for (int a = 0; a < e.numAdds[perspective]; ++a) adds[a] = net.feature_weights[e.adds[perspective][a]].vals;
            for (int s = 0; s < e.numSubs[perspective]; ++s) subs[s] = net.feature_weights[e.subs[perspective][s]].vals;
            updateAccumulator(e.acc[perspective].vals, g_stack[j - 1].acc[perspective].vals,
                              adds, e.numAdds[perspective], subs, e.numSubs[perspective]);
            e.computed[perspective] = true;
        }
    }
}

void nnue_reset(const uint64_t piece_bbs[2][6]) {
    g_top = 0;
    StackEntry& root = g_stack[0];
    for (int perspective = 0; perspective < 2; ++perspective) {
        root.kingSquare[perspective] = count_trailing_zeros(piece_bbs[perspective][5]);
        root.needsRefresh[perspective] = false;
        root.numAdds[perspective] = root.numSubs[perspective] = 0;
        refresh_cached(root.acc[perspective], perspective == 0, piece_bbs, root.kingSquare[perspective]);
        root.computed[perspective] = true;
    }
}

void nnue_push_move(const BoardDelta& delta, int whiteKingSquare, int blackKingSquare) {
    if (g_top + 1 >= STACK_SIZE) return; // can't happen within MAX_UNDO; keep the stack valid
    StackEntry& e = g_stack[++g_top];
    e.kingSquare[0] = whiteKingSquare;
    e.kingSquare[1] = blackKingSquare;
    for (int perspective = 0; perspective < 2; ++perspective) {
        e.computed[perspective] = false;
        e.needsRefresh[perspective] = delta.kingMoved[perspective];
        e.numAdds[perspective] = e.numSubs[perspective] = 0;
        if (delta.kingMoved[perspective]) continue;
        const bool white = perspective == 0;
        const int king = e.kingSquare[perspective];
        for (int k = 0; k < delta.numAdded; ++k) {
            const PieceChange& pc = delta.added[k];
            e.adds[perspective][e.numAdds[perspective]++] =
                static_cast<uint32_t>(halfka_feature_index(white, pc.white, pc.type, pc.square, king));
        }
        for (int k = 0; k < delta.numRemoved; ++k) {
            const PieceChange& pc = delta.removed[k];
            e.subs[perspective][e.numSubs[perspective]++] =
                static_cast<uint32_t>(halfka_feature_index(white, pc.white, pc.type, pc.square, king));
        }
        // Start loading the weight rows now; they're 2 KB each in a 46 MB table and
        // rarely cached, and the evaluation that needs them usually comes soon after.
        if (g_nnue_network) {
            for (int k = 0; k < e.numAdds[perspective]; ++k)
                prefetch_row(g_nnue_network->feature_weights[e.adds[perspective][k]].vals);
            for (int k = 0; k < e.numSubs[perspective]; ++k)
                prefetch_row(g_nnue_network->feature_weights[e.subs[perspective][k]].vals);
        }
    }
}

void nnue_pop() {
    if (g_top > 0) --g_top;
}

const Accumulator& nnue_current_accumulator(bool perspective, const uint64_t piece_bbs[2][6]) {
    materialize(perspective ? 0 : 1, piece_bbs);
    return g_stack[g_top].acc[perspective ? 0 : 1];
}

int32_t nnue_evaluate(bool whiteToMove, const uint64_t piece_bbs[2][6]) {
    materialize(0, piece_bbs);
    materialize(1, piece_bbs);
    const StackEntry& top = g_stack[g_top];
    return whiteToMove ? g_nnue_network->evaluate(top.acc[0], top.acc[1])
                       : g_nnue_network->evaluate(top.acc[1], top.acc[0]);
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
    configure_kernels(*net);
    reset_cache(*net);
    g_nnue_network = std::move(net);
    return true;
}