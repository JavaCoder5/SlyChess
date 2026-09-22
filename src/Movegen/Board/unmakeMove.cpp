#include "unmakeMove.h"
#include <iostream>

void unmakeMove(Move m)
{
    if (historyTop <= 0) return;
    UndoInfo ui = history[--historyTop];

    // Determine moving side: mover was the side that just moved, which is !turn
    bool moverWhite = !turn;

    if (enPassantSquare != -1)
    {
        boardHash ^= Zobrist::enpassant[enPassantSquare & 7];
    }

    int fromSq = m & 0x3F;
    int toSq = (m >> 6) & 0x3F;
    int flags = m & 0xF000;

    U64 fromBit = 1ULL << fromSq;
    U64 toBit = 1ULL << toSq;

    // Revert promotions
    if (ui.wasPromotion) {
        if (moverWhite) {
            if (flags == FLAG_PROMOTION_Q) { 
                wQueenBB &= ~toBit; 
                boardHash ^= Zobrist::psq[wQueen][toSq]; 
            }
            else if (flags == FLAG_PROMOTION_R) { 
                wRookBB &= ~toBit; 
                boardHash ^= Zobrist::psq[wRook][toSq]; 
            }
            else if (flags == FLAG_PROMOTION_B) { 
                wBishopBB &= ~toBit; 
                boardHash ^= Zobrist::psq[wBishop][toSq]; 
            }
            else if (flags == FLAG_PROMOTION_N) { 
                wKnightBB &= ~toBit; 
                boardHash ^= Zobrist::psq[wKnight][toSq]; 
            }
            wPawnBB |= fromBit; 
            boardHash ^= Zobrist::psq[wPawn][fromSq];
        }
        else {
            if (flags == FLAG_PROMOTION_Q) { 
                bQueenBB &= ~toBit; 
                boardHash ^= Zobrist::psq[bQueen][toSq];
            }
            else if (flags == FLAG_PROMOTION_R) { 
                bRookBB &= ~toBit; 
                boardHash ^= Zobrist::psq[bRook][toSq]; 
            }
            else if (flags == FLAG_PROMOTION_B) { 
                bBishopBB &= ~toBit; 
                boardHash ^= Zobrist::psq[bBishop][toSq]; 
            }
            else if (flags == FLAG_PROMOTION_N) { 
                bKnightBB &= ~toBit; 
                boardHash ^= Zobrist::psq[bKnight][toSq]; 
            }
            bPawnBB |= fromBit;
            boardHash ^= Zobrist::psq[bPawn][fromSq];
        }
        if (ui.capturedPiece) {
            U64 capBit = 1ULL << ui.capturedSquare;
            if (ui.capturedPiece == 'p') { 
                bPawnBB |= capBit; 
                boardHash ^= Zobrist::psq[bPawn][ui.capturedSquare]; 
            }
            else if (ui.capturedPiece == 'n') { 
                bKnightBB |= capBit; 
                boardHash ^= Zobrist::psq[bKnight][ui.capturedSquare]; 
            }
            else if (ui.capturedPiece == 'b') { 
                bBishopBB |= capBit; 
                boardHash ^= Zobrist::psq[bBishop][ui.capturedSquare]; 
            }
            else if (ui.capturedPiece == 'r') { 
                bRookBB |= capBit; 
                boardHash ^= Zobrist::psq[bRook][ui.capturedSquare]; 
            }
            else if (ui.capturedPiece == 'q') { 
                bQueenBB |= capBit; 
                boardHash ^= Zobrist::psq[bQueen][ui.capturedSquare]; 
            }
            else if (ui.capturedPiece == 'k') { 
                bKingBB |= capBit; 
                boardHash ^= Zobrist::psq[bKing][ui.capturedSquare]; 
            }
            else if (ui.capturedPiece == 'P') { 
                wPawnBB |= capBit; 
                boardHash ^= Zobrist::psq[wPawn][ui.capturedSquare]; 
            }
            else if (ui.capturedPiece == 'N') { 
                wKnightBB |= capBit; 
                boardHash ^= Zobrist::psq[wKnight][ui.capturedSquare]; 
            }
            else if (ui.capturedPiece == 'B') { 
                wBishopBB |= capBit; 
                boardHash ^= Zobrist::psq[wBishop][ui.capturedSquare]; 
            }
            else if (ui.capturedPiece == 'R') { 
                wRookBB |= capBit; 
                boardHash ^= Zobrist::psq[wRook][ui.capturedSquare]; 
            }
            else if (ui.capturedPiece == 'Q') { 
                wQueenBB |= capBit; 
                boardHash ^= Zobrist::psq[wQueen][ui.capturedSquare]; 
            }
            else if (ui.capturedPiece == 'K') { 
                wKingBB |= capBit; 
                boardHash ^= Zobrist::psq[wKing][ui.capturedSquare]; 
            }
        }
    }
    else if (ui.wasEnPassant) {
        if (moverWhite) {
            wPawnBB &= ~toBit; boardHash ^= Zobrist::psq[wPawn][toSq];
            U64 capBit = 1ULL << ui.capturedSquare;
            bPawnBB |= capBit; boardHash ^= Zobrist::psq[bPawn][ui.capturedSquare];
            wPawnBB |= fromBit; boardHash ^= Zobrist::psq[wPawn][fromSq];
        }
        else {
            bPawnBB &= ~toBit; boardHash ^= Zobrist::psq[bPawn][toSq];
            U64 capBit = 1ULL << ui.capturedSquare;
            wPawnBB |= capBit; boardHash ^= Zobrist::psq[wPawn][ui.capturedSquare];
            bPawnBB |= fromBit; boardHash ^= Zobrist::psq[bPawn][fromSq];
        }
    }
    else {
        if (moverWhite) {
            if (wPawnBB & toBit) { 
                wPawnBB &= ~toBit; 
                boardHash ^= Zobrist::psq[wPawn][toSq]; 
                wPawnBB |= fromBit; 
                boardHash ^= Zobrist::psq[wPawn][fromSq]; 
            }
            else if (wKnightBB & toBit) { 
                wKnightBB &= ~toBit; 
                boardHash ^= Zobrist::psq[wKnight][toSq]; 
                wKnightBB |= fromBit; 
                boardHash ^= Zobrist::psq[wKnight][fromSq]; 
            }
            else if (wBishopBB & toBit) { 
                wBishopBB &= ~toBit; 
                boardHash ^= Zobrist::psq[wBishop][toSq]; 
                wBishopBB |= fromBit; 
                boardHash ^= Zobrist::psq[wBishop][fromSq]; 
            }
            else if (wRookBB & toBit) { 
                wRookBB &= ~toBit; 
                boardHash ^= Zobrist::psq[wRook][toSq]; 
                wRookBB |= fromBit; 
                boardHash ^= Zobrist::psq[wRook][fromSq]; 
            }
            else if (wQueenBB & toBit) { 
                wQueenBB &= ~toBit; 
                boardHash ^= Zobrist::psq[wQueen][toSq]; 
                wQueenBB |= fromBit; 
                boardHash ^= Zobrist::psq[wQueen][fromSq];
            }
            else if (wKingBB & toBit) { 
                wKingBB &= ~toBit; 
                boardHash ^= Zobrist::psq[wKing][toSq]; 
                wKingBB |= fromBit; 
                boardHash ^= Zobrist::psq[wKing][fromSq]; 
            }

            if (ui.capturedPiece) {
                U64 capBit = 1ULL << ui.capturedSquare;
                if (ui.capturedPiece == 'p') { 
                    bPawnBB |= capBit; 
                    boardHash ^= Zobrist::psq[bPawn][ui.capturedSquare]; 
                }
                else if (ui.capturedPiece == 'n') { 
                    bKnightBB |= capBit; 
                    boardHash ^= Zobrist::psq[bKnight][ui.capturedSquare]; 
                }
                else if (ui.capturedPiece == 'b') { 
                    bBishopBB |= capBit; 
                    boardHash ^= Zobrist::psq[bBishop][ui.capturedSquare]; 
                }
                else if (ui.capturedPiece == 'r') { 
                    bRookBB |= capBit; 
                    boardHash ^= Zobrist::psq[bRook][ui.capturedSquare]; 
                }
                else if (ui.capturedPiece == 'q') { 
                    bQueenBB |= capBit; 
                    boardHash ^= Zobrist::psq[bQueen][ui.capturedSquare]; 
                }
                else if (ui.capturedPiece == 'k') { 
                    bKingBB |= capBit; 
                    boardHash ^= Zobrist::psq[bKing][ui.capturedSquare]; 
                }
            }

            if (ui.wasCastle && ui.rookFrom >= 0) {
                U64 rf = 1ULL << ui.rookFrom;
                U64 rt = 1ULL << ui.rookTo;
                if (wRookBB & rt) {
                    wRookBB &= ~rt; 
                    boardHash ^= Zobrist::psq[wRook][ui.rookTo]; 
                    wRookBB |= rf; 
                    boardHash ^= Zobrist::psq[wRook][ui.rookFrom];
                }
            }
        }
        else {
            if (bPawnBB & toBit) { 
                bPawnBB &= ~toBit; 
                boardHash ^= Zobrist::psq[bPawn][toSq]; 
                bPawnBB |= fromBit; 
                boardHash ^= Zobrist::psq[bPawn][fromSq]; 
            }
            else if (bKnightBB & toBit) { 
                bKnightBB &= ~toBit; 
                boardHash ^= Zobrist::psq[bKnight][toSq]; 
                bKnightBB |= fromBit; 
                boardHash ^= Zobrist::psq[bKnight][fromSq]; 
            }
            else if (bBishopBB & toBit) { 
                bBishopBB &= ~toBit; 
                boardHash ^= Zobrist::psq[bBishop][toSq]; 
                bBishopBB |= fromBit; 
                boardHash ^= Zobrist::psq[bBishop][fromSq]; 
            }
            else if (bRookBB & toBit) { 
                bRookBB &= ~toBit; 
                boardHash ^= Zobrist::psq[bRook][toSq]; 
                bRookBB |= fromBit; 
                boardHash ^= Zobrist::psq[bRook][fromSq]; 
            }
            else if (bQueenBB & toBit) { 
                bQueenBB &= ~toBit; 
                boardHash ^= Zobrist::psq[bQueen][toSq]; 
                bQueenBB |= fromBit; 
                boardHash ^= Zobrist::psq[bQueen][fromSq]; 
            }
            else if (bKingBB & toBit) { 
                bKingBB &= ~toBit; 
                boardHash ^= Zobrist::psq[bKing][toSq]; 
                bKingBB |= fromBit; 
                boardHash ^= Zobrist::psq[bKing][fromSq]; 
            }

            if (ui.capturedPiece) {
                U64 capBit = 1ULL << ui.capturedSquare;
                if (ui.capturedPiece == 'P') { 
                    wPawnBB |= capBit; 
                    boardHash ^= Zobrist::psq[wPawn][ui.capturedSquare]; 
                }
                else if (ui.capturedPiece == 'N') { 
                    wKnightBB |= capBit; 
                    boardHash ^= Zobrist::psq[wKnight][ui.capturedSquare]; 
                }
                else if (ui.capturedPiece == 'B') { 
                    wBishopBB |= capBit; 
                    boardHash ^= Zobrist::psq[wBishop][ui.capturedSquare]; 
                }
                else if (ui.capturedPiece == 'R') {
                    wRookBB |= capBit; 
                    boardHash ^= Zobrist::psq[wRook][ui.capturedSquare]; 
                }
                else if (ui.capturedPiece == 'Q') { 
                    wQueenBB |= capBit; 
                    boardHash ^= Zobrist::psq[wQueen][ui.capturedSquare]; 
                }
                else if (ui.capturedPiece == 'K') { 
                    wKingBB |= capBit; 
                    boardHash ^= Zobrist::psq[wKing][ui.capturedSquare]; 
                }
            }

            if (ui.wasCastle && ui.rookFrom >= 0) {
                U64 rf = 1ULL << ui.rookFrom;
                U64 rt = 1ULL << ui.rookTo;
                if (bRookBB & rt) { 
                    bRookBB &= ~rt; 
                    boardHash ^= Zobrist::psq[bRook][ui.rookTo]; 
                    bRookBB |= rf; 
                    boardHash ^= Zobrist::psq[bRook][ui.rookFrom]; 
                }
            }
        }
    }

    // Restore previous en-passant and castling rights
    enPassantSquare = ui.prevEnPassantSquare;

    if (enPassantSquare != -1)
    {
        boardHash ^= Zobrist::enpassant[enPassantSquare & 7];
    }

    // Update castling zobrist: remove current rights, apply previous rights
    int cr = 0 | (wKingCastleKRights ? 1 : 0) |
        (wKingCastleQRights ? (1 << 1) : 0) |
        (bKingCastleKRights ? (1 << 2) : 0) |
        (bKingCastleQRights ? (1 << 3) : 0);
    boardHash ^= Zobrist::castling[cr];

    wKingCastleKRights = ui.prev_wKR; wKingCastleQRights = ui.prev_wQR;
    bKingCastleKRights = ui.prev_bKR; bKingCastleQRights = ui.prev_bQR;

    cr = 0 | (wKingCastleKRights ? 1 : 0) |
        (wKingCastleQRights ? (1 << 1) : 0) |
        (bKingCastleKRights ? (1 << 2) : 0) |
        (bKingCastleQRights ? (1 << 3) : 0);
    boardHash ^= Zobrist::castling[cr];

    // Update aggregate bitboards
    allWhiteBB = wPawnBB | wKnightBB | wBishopBB | wRookBB | wQueenBB | wKingBB;
    allBlackBB = bPawnBB | bKnightBB | bBishopBB | bRookBB | bQueenBB | bKingBB;
    allPiecesBB = allWhiteBB | allBlackBB;

    // Flip side back to mover
    turn = !turn;
    boardHash ^= Zobrist::sideToMoveKey;
}