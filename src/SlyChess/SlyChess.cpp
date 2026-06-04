#include <iostream>
#include <string>
#include <sstream>
#include <bitset>
#include <src/Constants/Constants.h>
#include <src/Constants/Macros.h>
#include <src/Movegen/initKnightAttacks.h>
#include <src/Movegen/initKingAttacks.h>
#include <src/Movegen/initPawnAttacks.h>
#include <src/Movegen/MagicBoards/lookupAttacks.h>
#include <src/Movegen/MagicBoards/initSliders.h>
#include <src/Movegen/initPawnCaptures.h>

U64 wPawnBB = WPAWN_START;
U64 wKnightBB = WKNIGHT_START;
U64 wBishopBB = WBISHOP_START;
U64 wRookBB = WROOK_START;
U64 wQueenBB = WQUEEN_START;
U64 wKingBB = WKING_START;

U64 bPawnBB = BPAWN_START;
U64 bKnightBB = BKNIGHT_START;
U64 bBishopBB = BBISHOP_START;
U64 bRookBB = BROOK_START;
U64 bQueenBB = BQUEEN_START;
U64 bKingBB = BKING_START;

U64 allWhiteBB = wPawnBB | wKnightBB | wBishopBB | wRookBB | wQueenBB | wKingBB;
U64 allBlackBB = bPawnBB | bKnightBB | bBishopBB | bRookBB | bQueenBB | bKingBB;
U64 allPiecesBB = allWhiteBB | allBlackBB;

U64 wPawnAttacks[64];
U64 bPawnAttacks[64];
U64 wPawnCaptures[64];
U64 bPawnCaptures[64];
U64 knightAttacks[64];
U64 kingAttacks[64];

// Print bitboard as 8x8 grid (rank 8 at top, rank 1 at bottom).
static void printBitboard(U64 bb)
{
    for (int r = 7; r >= 0; --r) {
        for (int f = 0; f < 8; ++f) {
            int sq = r * 8 + f;
            std::cout << (((bb >> sq) & 1ULL) ? '1' : '.');
            std::cout << " ";
        }
        std::cout << "\n";
    }
}

void initBitboardAttacks()
{
    initWPawnAttacks(&wPawnAttacks);
	initBPawnAttacks(&bPawnAttacks);
	initWPawnCaptures(&wPawnCaptures);
	initBPawnCaptures(&bPawnCaptures);
    initKnightAttacks(&knightAttacks);
    initKingAttacks(&kingAttacks);
    return;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string line;

    initBitboardAttacks();
    initSliders();

    while (std::getline(std::cin, line)) {

        if (line == "uci") {
            std::cout << "id name SlyChess\n";
            std::cout << "id author JavaCoder5\n";
            std::cout << "uciok\n" << std::flush;
        }
        else if (line == "isready") {
            std::cout << "readyok\n" << std::flush;
        }
        else if (line == "ucinewgame") {
            
        }
        else if (line.rfind("position", 0) == 0) {
            // Always stop any running search before changing the board

        }
        else if (line.rfind("go", 0) == 0) {
            // Parse depth (only "go depth X" for now)
            int depth = 10;
            {
                std::stringstream ss(line);
                std::string token;
                ss >> token; // "go"
                if (ss >> token && token == "depth")
                    ss >> depth;
                else if (token == "infinite")
                    depth = INF;
            }
        }
        else if (line == "eval") {
            // int score = evaluate();
            // std::cout << "Evaluation: " << score << "\n" << std::flush;
        }
        else if (line.rfind("legals", 0) == 0) {

        }
        else if (line.rfind("all", 0) == 0) {

        }
        else if (line.rfind("perft", 0) == 0) {

        }
        else if (line.rfind("divide", 0) == 0) {

        }
        else if (line == "s") {
            // Print the current board state (for testing purposes)
            std::cout << "Current board state:\n" << std::flush;
            std::cout << "White Pawns:   \n";
            printBitboard(wPawnBB);
            std::cout << std::flush;

            std::cout << "White Knights: \n";
            printBitboard(wKnightBB);
            std::cout << std::flush;

            std::cout << "White Bishops: \n";
            printBitboard(wBishopBB);
            std::cout << std::flush;

            std::cout << "White Rooks:   \n";
            printBitboard(wRookBB);
            std::cout << std::flush;

            std::cout << "White Queens:  \n";
            printBitboard(wQueenBB);
            std::cout << std::flush;

            std::cout << "White King:    \n";
            printBitboard(wKingBB);
            std::cout << std::flush;

            std::cout << "Black Pawns:   \n";
            printBitboard(bPawnBB);
            std::cout << std::flush;

            std::cout << "Black Knights: \n";
            printBitboard(bKnightBB);
            std::cout << std::flush;

            std::cout << "Black Bishops: \n";
            printBitboard(bBishopBB);
            std::cout << std::flush;

            std::cout << "Black Rooks:   \n";
            printBitboard(bRookBB);
            std::cout << std::flush;

            std::cout << "Black Queens:  \n";
            printBitboard(bQueenBB);
            std::cout << std::flush;

            std::cout << "Black King:    \n";
            printBitboard(bKingBB);
            std::cout << std::flush;

            std::cout << "White Pieces:  \n";
            printBitboard(allWhiteBB);
            std::cout << std::flush;

            std::cout << "Black Pieces:  \n";
            printBitboard(allBlackBB);
            std::cout << std::flush;

            std::cout << "All Pieces:    \n";
            printBitboard(allPiecesBB);
            std::cout << std::flush;
        }
        else if (line.rfind("nmoves", 0) == 0) {
            std::stringstream ss(line);
            std::string token;
            int square = 0;
            ss >> token; // "nmoves"

            ss >> square;
            // Test knight move generation

            if (square < 0 || square > 63) {
                std::cout << "Invalid square index. Must be between 0 and 63.\n" << std::flush;
                continue;
            }

            std::cout << "Knight attacks from square " << square << ":\n";
            printBitboard(knightAttacks[square]);
            std::cout << std::flush;
        }
        else if (line.rfind("kmoves", 0) == 0) {
            std::stringstream ss(line);
            std::string token;
            int square = 0;
            ss >> token; // "kmoves"

            ss >> square;
            // Test king move generation

            if (square < 0 || square > 63) {
                std::cout << "Invalid square index. Must be between 0 and 63.\n" << std::flush;
                continue;
            }

            std::cout << "King attacks from square " << square << ":\n";
            printBitboard(kingAttacks[square]);
            std::cout << std::flush;
        }
        else if (line.rfind("wpmoves", 0) == 0) {
			std::stringstream ss(line);
			std::string token;
			int square = 0;
			ss >> token; // "wpmoves"
			ss >> square;
			// Test white pawn move generation
			if (square < 0 || square > 63) {
				std::cout << "Invalid square index. Must be between 0 and 63.\n" << std::flush;
				continue;
			}
			std::cout << "White pawn attacks from square " << square << ":\n";
			printBitboard(wPawnAttacks[square]);
			std::cout << std::flush;
        }
        else if (line.rfind("bpmoves", 0) == 0) {
            std::stringstream ss(line);
            std::string token;
            int square = 0;
            ss >> token; // "bpmoves"
            ss >> square;
            // Test black pawn move generation
            if (square < 0 || square > 63) {
                std::cout << "Invalid square index. Must be between 0 and 63.\n" << std::flush;
                continue;
            }
            std::cout << "Black pawn attacks from square " << square << ":\n";
            printBitboard(bPawnAttacks[square]);
            std::cout << std::flush;
        }
        else if (line.rfind("rmoves", 0) == 0) {
            std::stringstream ss(line);
            std::string token;
            int square = 0;
            ss >> token; // "rmoves"
            ss >> square;
            // Test rook move generation
            if (square < 0 || square > 63) {
                std::cout << "Invalid square index. Must be between 0 and 63.\n" << std::flush;
                continue;
            }
            std::cout << "Rook attacks from square " << square << ":\n";
            printBitboard(lookupRookAttacks(square, allPiecesBB) & ~allWhiteBB);
            std::cout << std::flush;
        }
        else if (line.rfind("bmoves", 0) == 0) {
            std::stringstream ss(line);
            std::string token;
            int square = 0;
            ss >> token; // "bmoves"
            ss >> square;
            // Test bishop move generation
            if (square < 0 || square > 63) {
                std::cout << "Invalid square index. Must be between 0 and 63.\n" << std::flush;
                continue;
            }
            std::cout << "Bishop attacks from square " << square << ":\n";
            printBitboard(lookupBishopAttacks(square, allPiecesBB) & ~allWhiteBB);
            std::cout << std::flush;
        }
        else if (line.rfind("qmoves", 0) == 0) {
            std::stringstream ss(line);
            std::string token;
            int square = 0;
            ss >> token; // "qmoves"
            ss >> square;
            // Test queen move generation
            if (square < 0 || square > 63) {
                std::cout << "Invalid square index. Must be between 0 and 63.\n" << std::flush;
                continue;
            }
            std::cout << "Queen attacks from square " << square << ":\n";
            printBitboard(lookupQueenAttacks(square, allPiecesBB) & ~allWhiteBB);
            std::cout << std::flush;
        }
        else if (line.rfind("wpcaps", 0) == 0) {
            std::stringstream ss(line);
            std::string token;
            int square = 0;
            ss >> token; // "wpcaps"
            ss >> square;
            // Test white pawn move generation
            if (square < 0 || square > 63) {
                std::cout << "Invalid square index. Must be between 0 and 63.\n" << std::flush;
                continue;
            }
            std::cout << "White pawn attacks from square " << square << ":\n";
            printBitboard(wPawnCaptures[square]);
            std::cout << std::flush;
        }
        else if (line.rfind("bpcaps", 0) == 0) {
            std::stringstream ss(line);
            std::string token;
            int square = 0;
            ss >> token; // "bpcaps"
            ss >> square;
            // Test black pawn move generation
            if (square < 0 || square > 63) {
                std::cout << "Invalid square index. Must be between 0 and 63.\n" << std::flush;
                continue;
            }
            std::cout << "Black pawn attacks from square " << square << ":\n";
            printBitboard(bPawnCaptures[square]);
            std::cout << std::flush;
        }
        else if (line.rfind("stop", 0) == 0) {

        }
        else if (line == "captures")
        {
            // Generate and print capture moves only (for testing quiescence move generation)

        }
        else if (line == "quit") {
                break;
        }
    }

    return 0;
}
