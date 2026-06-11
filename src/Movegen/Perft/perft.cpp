#include <src/Movegen/Perft/perft.h>

U64 perft(int depth)
{
    if (depth == 0) return 1ULL;
    Move moves[256] = { 0 };
    int moveCount = 0;
    generateLegalMoves(&moves, turn, &moveCount);
    U64 nodes = 0ULL;
    for (int i = 0; i < moveCount; ++i) {
        Move m = moves[i];
        if (m == 0) break;
        makeMove(m);
        nodes += perft(depth - 1);
        unmakeMove(m);
    }
    return nodes;
}

// Perft divide: prints per-root move node counts for given depth
void perftDivide(int depth)
{
    if (depth <= 0) {
        std::cout << "perftDivide: depth must be >= 1\n" << std::flush;
        return;
    }

    Move moves[256] = { 0 };
    int moveCount = 0;
    generateLegalMoves(&moves, turn, &moveCount);

    U64 total = 0ULL;
    for (int i = 0; i < moveCount; ++i) {
        Move m = moves[i];
        if (m == 0) break;
        makeMove(m);
        U64 cnt = perft(depth - 1);
        unmakeMove(m);
        std::cout << moveToUCI(m) << ": " << cnt << "\n" << std::flush;
        total += cnt;
    }
    std::cout << "Total: " << total << "\n" << std::flush;
}