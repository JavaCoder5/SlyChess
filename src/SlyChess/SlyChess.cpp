#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <chrono>
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
#include <src/Movegen/Board/makeMove.h>
#include <src/Movegen/generateLegalMoves.h>
#include <src/Movegen/Board/unmakeMove.h>
#include <src/uci/uciToMove.h>
#include <src/Movegen/Perft/perft.h>
#include <src/Search/search.h>
#include <src/Movegen/generateLegalCaptures.h>
#include <src/Search/stopAndJoinSearch.h>
#include <thread>
#include <src/timeManagement/searchTime.h>
#include <src/misc/printMutex.h>

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

// Fixed-size history to avoid dynamic allocation
UndoInfo history[MAX_UNDO];
int historyTop = 0; // next free index

//int problemMoveCount = 0;

int ply = 0;

std::atomic_bool searchRunning = false;
std::atomic_bool stopSearch = false;

std::thread searchThread;
std::thread timerThread;

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

    wKingCastleKRights = false;
    wKingCastleQRights = false;
    bKingCastleKRights = false;
    bKingCastleQRights = false;

	if (!castlingField.empty() && castlingField != "-") {
		wKingCastleQRights = castlingField.find('Q') != std::string::npos;
		wKingCastleKRights = castlingField.find('K') != std::string::npos;
        bKingCastleQRights = castlingField.find('q') != std::string::npos;
        bKingCastleKRights = castlingField.find('k') != std::string::npos;
	}

    return true;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string line;

    initBitboardAttacks();
    initSliders();

    while (std::getline(std::cin, line)) {

        if (line == "uci") {
            lockPrintMutex();
            std::cout << "id name SlyChess\n";
            std::cout << "id author JavaCoder5\n";
            std::cout << "id version 1.0a\n";
            std::cout << "uciok\n" << std::flush;
            unlockPrintMutex();
        }
        else if (line == "isready") {
            lockPrintMutex();
            std::cout << "readyok\n" << std::flush;
            unlockPrintMutex();
        }
        else if (line == "ucinewgame") {
            stopAndJoinSearch();

            setPositionFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        }
        else if (line.rfind("position", 0) == 0) {
            // Always stop any running search before changing the board
            std::stringstream ss(line);
            std::string token;
            ss >> token; // "position"
            std::string sub;
            if (!(ss >> sub)) continue;

            while (historyTop > 0)
                history[--historyTop] = UndoInfo(); // reset to default to avoid residual data and memory overflows

            if (sub == "startpos") {
                // set to standard start position
                setPositionFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

                // Check for optional moves following startpos
                std::string token;
                if (ss >> token && token == "moves") {
                    std::string mv;
                    while (ss >> mv) {
                        Move m = UCIToMove(mv);
                        if (m != 0) makeMove(m);
                    }
                }
            }
            else if (sub == "fen") {
                // remainder of the line after 'fen ' is the FEN string plus optional moves
                std::string fenRest;
                std::getline(ss, fenRest);
                // trim leading spaces
                size_t p = fenRest.find_first_not_of(' ');
                if (p != std::string::npos) fenRest = fenRest.substr(p);
                if (!fenRest.empty()) {
                    // Tokenize fenRest. FEN has up to 6 space-separated fields. After that optional 'moves' may follow.
                    std::stringstream fss(fenRest);
                    std::vector<std::string> parts;
                    std::string tmp;
                    while (fss >> tmp) parts.push_back(tmp);

                    size_t fenTokens = std::min((size_t)6, parts.size());
                    std::string fenString;
                    for (size_t i = 0; i < fenTokens; ++i) {
                        if (i) fenString += " ";
                        fenString += parts[i];
                    }

                    if (!fenString.empty()) {
                        setPositionFromFEN(fenString);
                    }

                    // If there are extra tokens and the next token is 'moves', apply the moves that follow
                    if (parts.size() > fenTokens && parts[fenTokens] == "moves") {
                        for (size_t i = fenTokens + 1; i < parts.size(); ++i) {
                            Move m = UCIToMove(parts[i]);
                            if (m != 0) makeMove(m);
                        }
                    }
                }
            }

        }
        else if (line.rfind("go", 0) == 0) {
            // Parse depth (only "go depth X" for now)
            int depth = 5;
            std::stringstream ss(line);
            std::string token;
            ss >> token; // "go"
            int whiteTime = 0;
            int blackTime = 0;
            int whiteIncrement = 0;
            int blackIncrement = 0;
            int goTime = 0;
            int goMode = searchForDepth;

            while (ss >> token)
            {
                if (token == "depth")
                {
                    ss >> depth;
                    break;
                }
                else if (token == "infinite")
                {
                    depth = INF;
                    break;
                }
                else if (token == "wtime")
                {
                    ss >> token; // the amount of time
                    whiteTime = stoi(token);
                    goMode = searchForTimeControl;
                }
                else if (token == "btime")
                {
                    ss >> token; // the amount of time
                    blackTime = stoi(token);
                    goMode = searchForTimeControl;
                }
                else if (token == "winc")
                {
                    ss >> token; // increment
                    whiteIncrement = stoi(token);
                    goMode = searchForTimeControl;
                }
                else if (token == "binc")
                {
                    ss >> token;
                    blackIncrement = stoi(token);
                    goMode = searchForTimeControl;
                }
                else if (token == "movetime")
                {
                    ss >> token;
                    goTime = stoi(token);
                    goMode = searchForSetTime;
                }
                else
                {
					std::cerr << "Unknown 'go' parameter: " << token << std::endl;
                    
                    continue;
                }
            }

            // Stop current search before starting a new one
            stopAndJoinSearch();

            if (goMode == searchForDepth)
            {
                searchThread = std::thread(search, depth);
            }
            else if (goMode == searchForTimeControl)
            {
                searchThread = std::thread(search, INF);
                timerThread = std::thread(searchTimeControl, whiteTime, blackTime, whiteIncrement, blackIncrement);
                timerThread.detach();
            }
            else if (goMode == searchForSetTime)
            {
                searchThread = std::thread(search, INF);
                timerThread = std::thread(searchTime, goTime);
                timerThread.detach();
            }

        }
        else if (line == "eval") {
            int score = evaluate();
            std::cout << "Evaluation: " << score << "\n" << std::flush;
        }
        else if (line.rfind("legals", 0) == 0) {
            std::cout << "Generating all legal moves for the current position...\n" << std::flush;

            int arraySize = 0;

            Move moves[256] = { 0 };
            generateLegalMoves(&moves, turn, &arraySize);

            int moveCounter = 0;
            while (true)
            {
                if (moves[moveCounter] == 0) break;
                std::cout << "Generated move: " << moveToUCI(moves[moveCounter]) << "\n" << std::flush;
                moveCounter++;
                if (moveCounter >= 256) break;
            }
        }
        else if (line.rfind("all", 0) == 0) {
			std::cout << "Generating all pseudo-legal moves for the current position...\n" << std::flush;

            int arraySize = 0;

            Move moves[256] = { 0 };
	        generatePseudoLegalMoves(&moves, turn, &arraySize);

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
            // Parse depth (only "go depth X" for now)
            int depth = 2;
            std::stringstream ss(line);
            std::string token;
            ss >> token; // "perft"

            if (ss >> token && token == "depth")
                ss >> depth;

			auto start = std::chrono::high_resolution_clock::now();
			U64 perftResult = perft(depth);
			auto end = std::chrono::high_resolution_clock::now();

			switch (depth) {
				case 1: 
                    std::cout << ((perftResult == 20) ? "PASSED: " : "FAIL: ") << "Perft at depth " << depth << ": " << perftResult << " (expected 20)" << std::endl;
                    break;
				case 2: 
                    std::cout << ((perftResult == 400) ? "PASSED: " : "FAIL: ") << "Perft at depth " << depth << ": " << perftResult << " (expected 400)" << std::endl;
                    break;
				case 3: 
                    std::cout << ((perftResult == 8902) ? "PASSED: " : "FAIL: ") << "Perft at depth " << depth << ": " << perftResult << " (expected 8902)" << std::endl;
                    break;
				case 4: 
                    std::cout << ((perftResult == 197281) ? "PASSED: " : "FAIL: ") << "Perft at depth " << depth << ": " << perftResult << " (expected 197281)" << std::endl;
                    break;
				case 5: 
                    std::cout << ((perftResult == 4865609) ? "PASSED: " : "FAIL: ") << "Perft at depth " << depth << ": " << perftResult << " (expected 4865609)" << std::endl;
                    break;
                case 6:
                    std::cout << ((perftResult == 119060324) ? "PASSED: " : "FAIL: ") << "Perft at depth " << depth << ": " << perftResult << " (expected 119060324)" << std::endl;
                    break;
				default: 
					std::cout << "Perft at depth " << depth << ": " << perftResult << " (no expected value for this depth)" << std::endl;
                    break;
	        }

			auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
			std::cout << "Time taken: " << duration << " ns (" << duration / 1000000 << " ms)\n" << std::flush;
			std::cout << "Nodes per second: " << (perftResult * 1000000000) / (duration > 0 ? duration : 1) << " n/s\n" << std::flush;

        }
        else if (line.rfind("divide", 0) == 0) {
            int depth = 3;
            std::stringstream ss(line);
            std::string token;

            ss >> token; // "divide"

            if (ss >> token && token == "depth")
                ss >> depth;

            perftDivide(depth);
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
            stopAndJoinSearch();
        }
        else if (line == "captures")
        {
            // Generate and print capture moves only (for testing quiescence move generation)
            std::cout << "Generating all captures for the current position...\n" << std::flush;

            int arraySize = 0;

            Move moves[256] = { 0 };
            generateLegalCaptures(&moves, turn, &arraySize);

            int moveCounter = 0;
            while (true)
            {
                if (moves[moveCounter] == 0) break;
                std::cout << "Generated move: " << moveToUCI(moves[moveCounter]) << "\n" << std::flush;
                moveCounter++;
                if (moveCounter >= 256) break;
            }
        }
        else if (line == "sstp")
        {
			turn = !turn; // Toggle turn for testing purposes
        }
        else if (line == "undo") {
            if (historyTop <= 0) {
                std::cout << "Nothing to undo\n" << std::flush;
            } else {
                Move last = history[historyTop-1].move;
                unmakeMove(last);
            }
        }
        else if (line.rfind("invert", 0) == 0)
        {
            std::stringstream ss(line);
            std::string token;
            int square = 0;
            ss >> token; // "invert"
            ss >> square;

            std::cout << square << " inverted is " << (8 - (square / 8)) + (square % 8) << std::endl;
        }
        else if (line == "quit") {
                break;
        }
        else {
            lockPrintMutex();
            std::cerr << "Unknown command: " << line << "\n" << std::flush;
            unlockPrintMutex();
        }

    }

    return 0;
}
