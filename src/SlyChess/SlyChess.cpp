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
#include <src/Movegen/generatePseudoLegalMoves.h>

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

bool turn = WHITE; // true for white to move, false for black to move
int enPassantSquare = -1; // -1 when no en-passant target

bool wKingCastleKRights = true;
bool wKingCastleQRights = true;

bool bKingCastleKRights = true;
bool bKingCastleQRights = true;

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

// Print the current position using piece letters (white: R N B K Q P, black: r n b k q p)
static void printPosition()
{
    for (int r = 7; r >= 0; --r) {
        for (int f = 0; f < 8; ++f) {
            int sq = r * 8 + f;
            U64 bit = 1ULL << sq;
            char c = '.';

            if (wRookBB & bit) c = 'R';
            else if (wKnightBB & bit) c = 'N';
            else if (wBishopBB & bit) c = 'B';
            else if (wKingBB & bit) c = 'K';
            else if (wQueenBB & bit) c = 'Q';
            else if (wPawnBB & bit) c = 'P';
            else if (bRookBB & bit) c = 'r';
            else if (bKnightBB & bit) c = 'n';
            else if (bBishopBB & bit) c = 'b';
            else if (bKingBB & bit) c = 'k';
            else if (bQueenBB & bit) c = 'q';
            else if (bPawnBB & bit) c = 'p';

            std::cout << c << " ";
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

// Set the global piece bitboards from a FEN string (only handles piece placement field
// for now). If fen is invalid the function returns false and does not modify state.
bool setPositionFromFEN(const std::string &fen)
{
    // Parse FEN fields: placement, side, castling, en-passant, halfmove, fullmove
    std::stringstream fs(fen);
    std::string placement;
    if (!(fs >> placement)) return false;
    std::string sideField;
    std::string castlingField;
    std::string epField;
    // Read optional fields if present
    fs >> sideField; // 'w' or 'b'
    fs >> castlingField; // may be '-'
    fs >> epField; // en-passant target or '-'

    // Prepare temporary bitboards
    U64 twPawn = 0ULL, twKnight = 0ULL, twBishop = 0ULL, twRook = 0ULL, twQueen = 0ULL, twKing = 0ULL;
    U64 tbPawn = 0ULL, tbKnight = 0ULL, tbBishop = 0ULL, tbRook = 0ULL, tbQueen = 0ULL, tbKing = 0ULL;

    int rank = 7;
    int file = 0;

    for (size_t i = 0; i < placement.size(); ++i) {
        char c = placement[i];
        if (c == '/') {
            if (file != 8) return false; // invalid
            --rank;
            file = 0;
            continue;
        }
        if (c >= '1' && c <= '8') {
            file += (c - '0');
            if (file > 8) return false;
            continue;
        }

        if (file >= 8 || rank < 0) return false;

        int sq = rank * 8 + file;
        U64 bit = 1ULL << sq;

        switch (c) {
            case 'P': twPawn |= bit; break;
            case 'N': twKnight |= bit; break;
            case 'B': twBishop |= bit; break;
            case 'R': twRook |= bit; break;
            case 'Q': twQueen |= bit; break;
            case 'K': twKing |= bit; break;
            case 'p': tbPawn |= bit; break;
            case 'n': tbKnight |= bit; break;
            case 'b': tbBishop |= bit; break;
            case 'r': tbRook |= bit; break;
            case 'q': tbQueen |= bit; break;
            case 'k': tbKing |= bit; break;
            default:
                return false; // unknown char
        }
        ++file;
    }

    if (rank != 0 || file != 8) {
        // After parsing the last rank we expect to have filled rank 0 and file==8
        // but some valid FENs may end exactly at that state; ensure consistency
        // If not consistent, still accept when rank==0 and file==8
        if (!(rank == 0 && file == 8)) return false;
    }

    // Commit to globals
    wPawnBB = twPawn;
    wKnightBB = twKnight;
    wBishopBB = twBishop;
    wRookBB = twRook;
    wQueenBB = twQueen;
    wKingBB = twKing;

    bPawnBB = tbPawn;
    bKnightBB = tbKnight;
    bBishopBB = tbBishop;
    bRookBB = tbRook;
    bQueenBB = tbQueen;
    bKingBB = tbKing;

    allWhiteBB = wPawnBB | wKnightBB | wBishopBB | wRookBB | wQueenBB | wKingBB;
    allBlackBB = bPawnBB | bKnightBB | bBishopBB | bRookBB | bQueenBB | bKingBB;
    allPiecesBB = allWhiteBB | allBlackBB;

    // Update side-to-move if provided
    if (!sideField.empty()) {
        if (sideField[0] == 'w') turn = WHITE;
        else if (sideField[0] == 'b') turn = BLACK;
    }

    // Parse en-passant square if provided
    enPassantSquare = -1;
    if (!epField.empty() && epField != "-") {
        if (epField.size() >= 2) {
            char f = epField[0];
            char r = epField[1];
            if (f >= 'a' && f <= 'h' && r >= '1' && r <= '8') {
                enPassantSquare = (r - '1') * 8 + (f - 'a');
            }
        }
    }

    return true;
}

std::string moveToUCI(Move move)
{
	int fromSq = move & 0x3F; // bits 0-5
	int toSq = (move >> 6) & 0x3F; // bits 6-11
	int flags = move & 0xF000; // bits 12-15 (if needed for special move handling)
	char fromFile = 'a' + (fromSq % 8);
	char fromRank = '1' + (fromSq / 8);
	char toFile = 'a' + (toSq % 8);
	char toRank = '1' + (toSq / 8);
	char promotionSuffix;
	switch (flags) {
	case FLAG_PROMOTION_Q: promotionSuffix = 'q'; break;
	case FLAG_PROMOTION_R: promotionSuffix = 'r'; break;
	case FLAG_PROMOTION_B: promotionSuffix = 'b'; break;
	case FLAG_PROMOTION_N: promotionSuffix = 'n'; break;
	default: promotionSuffix = '\0'; break; // No promotion
	}

	return std::string() + fromFile + fromRank + toFile + toRank + promotionSuffix;
}

Move UCIToMove(std::string move)
{
	if (move.length() < 4) return 0; // Invalid move string
	char fromFile = move[0];
	char fromRank = move[1];
	char toFile = move[2];
	char toRank = move[3];
	if (fromFile < 'a' || fromFile > 'h' || toFile < 'a' || toFile > 'h' ||
		fromRank < '1' || fromRank > '8' || toRank < '1' || toRank > '8') {
		return 0; // Invalid characters
	}
	int fromSq = (fromRank - '1') * 8 + (fromFile - 'a');
	int toSq = (toRank - '1') * 8 + (toFile - 'a');
	return (fromSq) | (toSq << 6);
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
            std::stringstream ss(line);
            std::string token;
            ss >> token; // "position"
            std::string sub;
            if (!(ss >> sub)) continue;

            if (sub == "startpos") {
                // set to standard start position
                setPositionFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            }
            else if (sub == "fen") {
                // remainder of the line after 'fen ' is the FEN string
                std::string fenRest;
                std::getline(ss, fenRest);
                // trim leading spaces
                size_t p = fenRest.find_first_not_of(' ');
                if (p != std::string::npos) fenRest = fenRest.substr(p);
                if (!fenRest.empty()) {
                    setPositionFromFEN(fenRest);
                }
            }

            // ignore optional "moves" and move list for now

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
			std::cout << "Generating all pseudo-legal moves for the current position...\n" << std::flush;

            Move moves[256] = { 0 };
	        generatePseudoLegalMoves(&moves, turn);

            int moveCounter = 0;
            while (true)
            {
                if (moves[moveCounter] == 0) break;
				std::cout << "Generated move: " << moveToUCI(moves[moveCounter]) << "\n" << std::flush;
                moveCounter++;
				if (moveCounter >= 256) break;
            }

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

			std::cout << (turn ? "White to move\n" : "Black to move\n");
        }
        else if (line == "p") {
            // Print board with piece letters
            printPosition();
            std::cout << (turn ? "White to move\n" : "Black to move\n");
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
        else if (line == "sstp")
        {
			turn = !turn; // Toggle turn for testing purposes
        }
        else if (line == "quit") {
                break;
        }
    }

    return 0;
}
