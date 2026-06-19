#include <src/Movegen/Board/makeMove.h>

// Apply a move to the global board state. This updates piece bitboards,
// en-passant square, castling rights and flips the side-to-move (turn).
void makeMove(Move m)
{
    int fromSq = m & 0x3F;
    int toSq = (m >> 6) & 0x3F;
    int flags = m & 0xF000;

    U64 fromBit = 1ULL << fromSq;
    U64 toBit = 1ULL << toSq;

    // Clear en-passant by default; it will be set again on double pawn pushes
    enPassantSquare = -1;

    history[historyTop].wasEnPassant = false;

    history[historyTop].prevEnPassantSquare = enPassantSquare;

    bool whiteToMove = turn;

    history[historyTop].move = m;

    history[historyTop].capturedPiece = 0;

    history[historyTop].wasPromotion = false;

	history[historyTop].rookFrom = -1;
	history[historyTop].rookTo = -1;

    // Helper: clear any piece of the opponent on the destination square
    auto clearOpponentOnSquare = [&](int sq) {
        U64 bit = 1ULL << sq;
        if (whiteToMove) {
            if (bPawnBB & bit)
            {
                bPawnBB &= ~bit;
                history[historyTop].capturedPiece = 'p';
                history[historyTop].capturedSquare = sq;
                boardHash ^= Zobrist::psq[bPawn][sq];
            }
            if (bKnightBB & bit)
            {
                bKnightBB &= ~bit;
                history[historyTop].capturedPiece = 'n';
                history[historyTop].capturedSquare = sq;
                boardHash ^= Zobrist::psq[bKnight][sq];
            }
            if (bBishopBB & bit)
            {
                bBishopBB &= ~bit;
                history[historyTop].capturedPiece = 'b';
                history[historyTop].capturedSquare = sq;
                boardHash ^= Zobrist::psq[bBishop][sq];
            }
            if (bRookBB & bit)
            {
                bRookBB &= ~bit;
                history[historyTop].capturedPiece = 'r';
                history[historyTop].capturedSquare = sq;
                boardHash ^= Zobrist::psq[bRook][sq];
            }
            if (bQueenBB & bit)
            {
                bQueenBB &= ~bit;
                history[historyTop].capturedPiece = 'q';
                history[historyTop].capturedSquare = sq;
                boardHash ^= Zobrist::psq[bQueen][sq];
            }
            if (bKingBB & bit)
            {
                bKingBB &= ~bit;
                history[historyTop].capturedPiece = 'k';
                history[historyTop].capturedSquare = sq;
                boardHash ^= Zobrist::psq[bKing][sq];
            }
        }
        else {
            if (wPawnBB & bit)
            {
                wPawnBB &= ~bit;
                history[historyTop].capturedPiece = 'P';
                history[historyTop].capturedSquare = sq;
                boardHash ^= Zobrist::psq[wPawn][sq];
            }
            if (wKnightBB & bit)
            {
                wKnightBB &= ~bit;
                history[historyTop].capturedPiece = 'N';
                history[historyTop].capturedSquare = sq;
                boardHash ^= Zobrist::psq[wKnight][sq];
            }
            if (wBishopBB & bit)
            {
                wBishopBB &= ~bit;
                history[historyTop].capturedPiece = 'B';
                history[historyTop].capturedSquare = sq;
                boardHash ^= Zobrist::psq[wBishop][sq];
            }
            if (wRookBB & bit)
            {
                wRookBB &= ~bit;
                history[historyTop].capturedPiece = 'R';
                history[historyTop].capturedSquare = sq;
                boardHash ^= Zobrist::psq[wRook][sq];
            }
            if (wQueenBB & bit)
            {
                wQueenBB &= ~bit;
                history[historyTop].capturedPiece = 'Q';
                history[historyTop].capturedSquare = sq;
                boardHash ^= Zobrist::psq[wQueen][sq];
            }
            if (wKingBB & bit)
            {
                wKingBB &= ~bit;
                history[historyTop].capturedPiece = 'K';
                history[historyTop].capturedSquare = sq;
                boardHash ^= Zobrist::psq[wKing][sq];
            }
        }
        };

    // Handle en-passant capture
    if (flags == FLAG_EN_PASSANT) {
        int capturedPawnSq = whiteToMove ? (toSq - 8) : (toSq + 8);
        if (capturedPawnSq >= 0 && capturedPawnSq < 64) {
            U64 capBit = 1ULL << capturedPawnSq;
            if (whiteToMove)
            {
                bPawnBB &= ~capBit;
                boardHash ^= Zobrist::psq[bPawn][capturedPawnSq];
            }
            else
            {
                wPawnBB &= ~capBit;
                boardHash ^= Zobrist::psq[wPawn][capturedPawnSq];
            }
        }
        history[historyTop].wasEnPassant = true;
        history[historyTop].capturedPiece = whiteToMove ? 'p' : 'P';
        history[historyTop].capturedSquare = capturedPawnSq;
    }

    // Identify and move the piece from 'fromSq' to 'toSq'
    if (whiteToMove) {
        // White to move: check which white piece is on fromSq
        if (wPawnBB & fromBit) {
            // Promotion handling
            if (flags == FLAG_PROMOTION_Q || flags == FLAG_PROMOTION_R || flags == FLAG_PROMOTION_B || flags == FLAG_PROMOTION_N) {
                // remove pawn
                wPawnBB &= ~fromBit;
                boardHash ^= Zobrist::psq[wPawn][fromSq];
                // clear any captured piece on toSq
                clearOpponentOnSquare(toSq);
                // place promoted piece
                if (flags == FLAG_PROMOTION_Q)
                {
                    wQueenBB |= toBit;
                    boardHash ^= Zobrist::psq[wQueen][toSq];
                }
                else if (flags == FLAG_PROMOTION_R)
                {
                    wRookBB |= toBit;
                    boardHash ^= Zobrist::psq[wRook][toSq];
                }
                else if (flags == FLAG_PROMOTION_B)
                {
                    wBishopBB |= toBit;
                    boardHash ^= Zobrist::psq[wBishop][toSq];
                }
                else if (flags == FLAG_PROMOTION_N)
                {
                    wKnightBB |= toBit;
                    boardHash ^= Zobrist::psq[wKnight][toSq];
                }

                history[historyTop].wasPromotion = true;
            }
            else {
                // Normal pawn move
                wPawnBB &= ~fromBit;
                boardHash ^= Zobrist::psq[wPawn][fromSq];
                // capture if any
                if (!(flags == FLAG_EN_PASSANT)) clearOpponentOnSquare(toSq);
                wPawnBB |= toBit;
                boardHash ^= Zobrist::psq[wPawn][toSq];

                // If pawn moved two squares, set en-passant target
                int fromRank = fromSq / 8;
                int toRank = toSq / 8;
                if (toRank - fromRank == 2) {
                    enPassantSquare = fromSq + 8;
                    history[historyTop].prevEnPassantSquare = enPassantSquare;
                }
            }
        }
        else if (wKnightBB & fromBit) {
            boardHash ^= Zobrist::psq[wKnight][fromSq];
            wKnightBB &= ~fromBit; clearOpponentOnSquare(toSq); wKnightBB |= toBit;
            boardHash ^= Zobrist::psq[wKnight][toSq];
        }
        else if (wBishopBB & fromBit) {
            boardHash ^= Zobrist::psq[wBishop][fromSq];
            wBishopBB &= ~fromBit; clearOpponentOnSquare(toSq); wBishopBB |= toBit;
            boardHash ^= Zobrist::psq[wBishop][toSq];
        }
        else if (wRookBB & fromBit) {
            boardHash ^= Zobrist::psq[wRook][fromSq];
            wRookBB &= ~fromBit; clearOpponentOnSquare(toSq); wRookBB |= toBit;
            boardHash ^= Zobrist::psq[wRook][toSq];
            // Update castling rights if rook moved from initial squares
            if (fromSq == 0)
            {
                int cr = 0 | (wKingCastleKRights ? 1 : 0) | 
                    (wKingCastleQRights ? (1 << 1) : 0) | 
                    (bKingCastleKRights ? (1 << 2) : 0) |
                    (bKingCastleQRights ? (1 << 3) : 0);
                boardHash ^= Zobrist::castling[cr];
                wKingCastleQRights = false; // a1
                cr = 0 | (wKingCastleKRights ? 1 : 0) |
                    (wKingCastleQRights ? (1 << 1) : 0) |
                    (bKingCastleKRights ? (1 << 2) : 0) |
                    (bKingCastleQRights ? (1 << 3) : 0);
                boardHash ^= Zobrist::castling[cr];
            }
            else if (fromSq == 7)
            {
                int cr = 0 | (wKingCastleKRights ? 1 : 0) |
                    (wKingCastleQRights ? (1 << 1) : 0) |
                    (bKingCastleKRights ? (1 << 2) : 0) |
                    (bKingCastleQRights ? (1 << 3) : 0);
                boardHash ^= Zobrist::castling[cr];
                wKingCastleKRights = false; // h1
                cr = 0 | (wKingCastleKRights ? 1 : 0) |
                    (wKingCastleQRights ? (1 << 1) : 0) |
                    (bKingCastleKRights ? (1 << 2) : 0) |
                    (bKingCastleQRights ? (1 << 3) : 0);
                boardHash ^= Zobrist::castling[cr];
            }
        }
        else if (wQueenBB & fromBit) {
            boardHash ^= Zobrist::psq[wQueen][fromSq];
            wQueenBB &= ~fromBit; clearOpponentOnSquare(toSq); wQueenBB |= toBit; 
            boardHash ^= Zobrist::psq[wQueen][toSq];
        }
        else if (wKingBB & fromBit) {
            boardHash ^= Zobrist::psq[wKing][fromSq];
            wKingBB &= ~fromBit; clearOpponentOnSquare(toSq); wKingBB |= toBit;
            boardHash ^= Zobrist::psq[wKing][toSq];
            // King moved: revoke castling rights
            int cr = 0 | (wKingCastleKRights ? 1 : 0) |
                (wKingCastleQRights ? (1 << 1) : 0) |
                (bKingCastleKRights ? (1 << 2) : 0) |
                (bKingCastleQRights ? (1 << 3) : 0);
            boardHash ^= Zobrist::castling[cr];
            wKingCastleKRights = false; wKingCastleQRights = false;
            cr = 0 | (wKingCastleKRights ? 1 : 0) |
                (wKingCastleQRights ? (1 << 1) : 0) |
                (bKingCastleKRights ? (1 << 2) : 0) |
                (bKingCastleQRights ? (1 << 3) : 0);
            boardHash ^= Zobrist::castling[cr];

            // Handle castling: either flagged or detected by king moving two squares
            if (flags == FLAG_CASTLE_K) {
                U64 h1 = 1ULL << 7;
                U64 f1 = 1ULL << 5;
                if (wRookBB & h1) { 
                    boardHash ^= Zobrist::psq[wRook][7];
                    wRookBB &= ~h1; wRookBB |= f1; 
                    boardHash ^= Zobrist::psq[wRook][5];
                    history[historyTop].rookFrom = 7;
                    history[historyTop].rookTo = 5;
                }
                history[historyTop].wasCastle = true;
            }
            else if (flags == FLAG_CASTLE_Q) {
                U64 a1 = 1ULL << 0;
                U64 d1 = 1ULL << 3;
                if (wRookBB & a1) { 
                    boardHash ^= Zobrist::psq[wRook][0];
                    wRookBB &= ~a1; wRookBB |= d1; 
                    boardHash ^= Zobrist::psq[wRook][3];
                    history[historyTop].rookFrom = 0;
					history[historyTop].rookTo = 3;
                }
                history[historyTop].wasCastle = true;
            }
            else {
                int diff = toSq - fromSq;
                if (diff == 2) {
                    // king side castling e1->g1: move h1->f1
                    U64 h1 = 1ULL << 7;
                    U64 f1 = 1ULL << 5;
                    if (wRookBB & h1) { wRookBB &= ~h1; wRookBB |= f1; }
                }
                else if (diff == -2) {
                    // queen side castling e1->c1: move a1->d1
                    U64 a1 = 1ULL << 0;
                    U64 d1 = 1ULL << 3;
                    if (wRookBB & a1) { wRookBB &= ~a1; wRookBB |= d1; }
                }
            }
        }
    }
    else {
        // Black to move
        if (bPawnBB & fromBit) {
            if (flags == FLAG_PROMOTION_Q || flags == FLAG_PROMOTION_R || flags == FLAG_PROMOTION_B || flags == FLAG_PROMOTION_N) {
                boardHash ^= Zobrist::psq[bPawn][fromSq];
                bPawnBB &= ~fromBit;
                clearOpponentOnSquare(toSq);
                if (flags == FLAG_PROMOTION_Q)
                {
                    boardHash ^= Zobrist::psq[bQueen][toSq];
                    bQueenBB |= toBit;
                }
                else if (flags == FLAG_PROMOTION_R)
                {
                    boardHash ^= Zobrist::psq[bRook][toSq];
                    bRookBB |= toBit;
                }
                else if (flags == FLAG_PROMOTION_B)
                {
                    boardHash ^= Zobrist::psq[bBishop][toSq];
                    bBishopBB |= toBit;
                }
                else if (flags == FLAG_PROMOTION_N)
                {
                    boardHash ^= Zobrist::psq[bKnight][toSq];
                    bKnightBB |= toBit;
                }

                history[historyTop].wasPromotion = true;
            }
            else {
                boardHash ^= Zobrist::psq[bPawn][fromSq];
                bPawnBB &= ~fromBit;
                if (!(flags == FLAG_EN_PASSANT)) clearOpponentOnSquare(toSq);
                bPawnBB |= toBit;
                boardHash ^= Zobrist::psq[bPawn][toSq];

                int fromRank = fromSq / 8;
                int toRank = toSq / 8;
                if (fromRank - toRank == 2) {
                    enPassantSquare = fromSq - 8;
                    history[historyTop].prevEnPassantSquare = enPassantSquare;
                }
            }
        }
        else if (bKnightBB & fromBit) {
            boardHash ^= Zobrist::psq[bKnight][fromSq];
            bKnightBB &= ~fromBit; clearOpponentOnSquare(toSq); bKnightBB |= toBit;
            boardHash ^= Zobrist::psq[bKnight][toSq];
        }
        else if (bBishopBB & fromBit) {
            boardHash ^= Zobrist::psq[bBishop][fromSq];
            bBishopBB &= ~fromBit; clearOpponentOnSquare(toSq); bBishopBB |= toBit;
            boardHash ^= Zobrist::psq[bBishop][toSq];
        }
        else if (bRookBB & fromBit) {
            boardHash ^= Zobrist::psq[bRook][fromSq];
            bRookBB &= ~fromBit; clearOpponentOnSquare(toSq); bRookBB |= toBit;
            boardHash ^= Zobrist::psq[bRook][toSq];
            if (fromSq == 56)
            {
                int cr = 0 | (wKingCastleKRights ? 1 : 0) |
                    (wKingCastleQRights ? (1 << 1) : 0) |
                    (bKingCastleKRights ? (1 << 2) : 0) |
                    (bKingCastleQRights ? (1 << 3) : 0);
                boardHash ^= Zobrist::castling[cr];
                bKingCastleQRights = false; // a8
                cr = 0 | (wKingCastleKRights ? 1 : 0) |
                    (wKingCastleQRights ? (1 << 1) : 0) |
                    (bKingCastleKRights ? (1 << 2) : 0) |
                    (bKingCastleQRights ? (1 << 3) : 0);
                boardHash ^= Zobrist::castling[cr];
            }
            else if (fromSq == 63)
            {
                int cr = 0 | (wKingCastleKRights ? 1 : 0) |
                    (wKingCastleQRights ? (1 << 1) : 0) |
                    (bKingCastleKRights ? (1 << 2) : 0) |
                    (bKingCastleQRights ? (1 << 3) : 0);
                boardHash ^= Zobrist::castling[cr];
                bKingCastleKRights = false; // h8
                cr = 0 | (wKingCastleKRights ? 1 : 0) |
                    (wKingCastleQRights ? (1 << 1) : 0) |
                    (bKingCastleKRights ? (1 << 2) : 0) |
                    (bKingCastleQRights ? (1 << 3) : 0);
                boardHash ^= Zobrist::castling[cr];
            }
        }
        else if (bQueenBB & fromBit) {
            boardHash ^= Zobrist::psq[bQueen][fromSq];
            bQueenBB &= ~fromBit; clearOpponentOnSquare(toSq); bQueenBB |= toBit;
            boardHash ^= Zobrist::psq[bQueen][toSq];
        }
        else if (bKingBB & fromBit) {
            boardHash ^= Zobrist::psq[bKing][fromSq];
            bKingBB &= ~fromBit; clearOpponentOnSquare(toSq); bKingBB |= toBit;
            boardHash ^= Zobrist::psq[bKing][toSq];

            int cr = 0 | (wKingCastleKRights ? 1 : 0) |
                (wKingCastleQRights ? (1 << 1) : 0) |
                (bKingCastleKRights ? (1 << 2) : 0) |
                (bKingCastleQRights ? (1 << 3) : 0);
            boardHash ^= Zobrist::castling[cr];
            bKingCastleKRights = false; bKingCastleQRights = false;
            cr = 0 | (wKingCastleKRights ? 1 : 0) |
                (wKingCastleQRights ? (1 << 1) : 0) |
                (bKingCastleKRights ? (1 << 2) : 0) |
                (bKingCastleQRights ? (1 << 3) : 0);
            boardHash ^= Zobrist::castling[cr];

            if (flags == FLAG_CASTLE_K) {
                U64 h8 = 1ULL << 63;
                U64 f8 = 1ULL << 61;
                if (bRookBB & h8) { 
                    boardHash ^= Zobrist::psq[wRook][63];
                    bRookBB &= ~h8; bRookBB |= f8;
                    boardHash ^= Zobrist::psq[wRook][61];
                    history[historyTop].rookFrom = 63;
                    history[historyTop].rookTo = 61;
                }
                history[historyTop].wasCastle = true;
            }
            else if (flags == FLAG_CASTLE_Q) {
                U64 a8 = 1ULL << 56;
                U64 d8 = 1ULL << 59;
                if (bRookBB & a8) { 
                    boardHash ^= Zobrist::psq[wRook][56];
                    bRookBB &= ~a8; bRookBB |= d8;
                    boardHash ^= Zobrist::psq[wRook][59];
                    history[historyTop].rookFrom = 56;
                    history[historyTop].rookTo = 59;
                    history[historyTop].wasCastle = true;
                }
            }
            else {
                int diff = toSq - fromSq;
                if (diff == 2) {
                    // black king side e8->g8: move h8->f8
                    U64 h8 = 1ULL << 63;
                    U64 f8 = 1ULL << 61;
                    if (bRookBB & h8) { bRookBB &= ~h8; bRookBB |= f8; }
                }
                else if (diff == -2) {
                    // black queen side e8->c8: move a8->d8
                    U64 a8 = 1ULL << 56;
                    U64 d8 = 1ULL << 59;
                    if (bRookBB & a8) { bRookBB &= ~a8; bRookBB |= d8; }
                }
            }
        }
    }

    // If a rook was captured on its starting square, update castling rights for that side
    // White rooks start at 0 (a1) and 7 (h1); black at 56 (a8) and 63 (h8)
    if ((bRookBB & (1ULL << 0)) == 0) { /* nothing */ }
    // Check captures on rook starting squares and update opponent rights
    if ((bRookBB & (1ULL << 0)) == 0) { /* a1 is white rook square, irrelevant for black */ }

    // Update aggregate bitboards
    allWhiteBB = wPawnBB | wKnightBB | wBishopBB | wRookBB | wQueenBB | wKingBB;
    allBlackBB = bPawnBB | bKnightBB | bBishopBB | bRookBB | bQueenBB | bKingBB;
    allPiecesBB = allWhiteBB | allBlackBB;

    // Flip side to move
    turn = !turn;

    boardHash ^= Zobrist::sideToMoveKey;

    history[historyTop].prev_bKR = bKingCastleKRights;
    history[historyTop].prev_bQR = bKingCastleQRights;
    history[historyTop].prev_wKR = wKingCastleKRights;
    history[historyTop].prev_wQR = wKingCastleQRights;

    historyTop++;
}