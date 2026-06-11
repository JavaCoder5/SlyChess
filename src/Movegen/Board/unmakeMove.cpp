#include "unmakeMove.h"

void unmakeMove(Move m)
{
    if (historyTop <= 0) return;
    UndoInfo ui = history[--historyTop];

    // Determine moving side: mover was the side that just moved, which is !turn
    bool moverWhite = !turn;

    int fromSq = m & 0x3F;
    int toSq = (m >> 6) & 0x3F;
    int flags = m & 0xF000;

    U64 fromBit = 1ULL << fromSq;
    U64 toBit = 1ULL << toSq;

    // Revert promotions
    if (ui.wasPromotion) {
        if (moverWhite) {
            if (flags == FLAG_PROMOTION_Q) wQueenBB &= ~toBit;
            else if (flags == FLAG_PROMOTION_R) wRookBB &= ~toBit;
            else if (flags == FLAG_PROMOTION_B) wBishopBB &= ~toBit;
            else if (flags == FLAG_PROMOTION_N) wKnightBB &= ~toBit;
            wPawnBB |= fromBit;
        }
        else {
            if (flags == FLAG_PROMOTION_Q) bQueenBB &= ~toBit;
            else if (flags == FLAG_PROMOTION_R) bRookBB &= ~toBit;
            else if (flags == FLAG_PROMOTION_B) bBishopBB &= ~toBit;
            else if (flags == FLAG_PROMOTION_N) bKnightBB &= ~toBit;
            bPawnBB |= fromBit;
        }
        if (ui.capturedPiece) {
            U64 capBit = 1ULL << ui.capturedSquare;
            if (ui.capturedPiece == 'p') bPawnBB |= capBit;
            else if (ui.capturedPiece == 'n') bKnightBB |= capBit;
            else if (ui.capturedPiece == 'b') bBishopBB |= capBit;
            else if (ui.capturedPiece == 'r') bRookBB |= capBit;
            else if (ui.capturedPiece == 'q') bQueenBB |= capBit;
            else if (ui.capturedPiece == 'k') bKingBB |= capBit;
            else if (ui.capturedPiece == 'P') wPawnBB |= capBit;
            else if (ui.capturedPiece == 'N') wKnightBB |= capBit;
            else if (ui.capturedPiece == 'B') wBishopBB |= capBit;
            else if (ui.capturedPiece == 'R') wRookBB |= capBit;
            else if (ui.capturedPiece == 'Q') wQueenBB |= capBit;
            else if (ui.capturedPiece == 'K') wKingBB |= capBit;
        }
    }
    else if (ui.wasEnPassant) {
        if (moverWhite) {
            wPawnBB &= ~toBit;
            U64 capBit = 1ULL << ui.capturedSquare;
            bPawnBB |= capBit;
            wPawnBB |= fromBit;
        }
        else {
            bPawnBB &= ~toBit;
            U64 capBit = 1ULL << ui.capturedSquare;
            wPawnBB |= capBit;
            bPawnBB |= fromBit;
        }
    }
    else {
        if (moverWhite) {
            if (wPawnBB & toBit) { wPawnBB &= ~toBit; wPawnBB |= fromBit; }
            else if (wKnightBB & toBit) { wKnightBB &= ~toBit; wKnightBB |= fromBit; }
            else if (wBishopBB & toBit) { wBishopBB &= ~toBit; wBishopBB |= fromBit; }
            else if (wRookBB & toBit) { wRookBB &= ~toBit; wRookBB |= fromBit; }
            else if (wQueenBB & toBit) { wQueenBB &= ~toBit; wQueenBB |= fromBit; }
            else if (wKingBB & toBit) { wKingBB &= ~toBit; wKingBB |= fromBit; }

            if (ui.capturedPiece) {
                U64 capBit = 1ULL << ui.capturedSquare;
                if (ui.capturedPiece == 'p') bPawnBB |= capBit;
                else if (ui.capturedPiece == 'n') bKnightBB |= capBit;
                else if (ui.capturedPiece == 'b') bBishopBB |= capBit;
                else if (ui.capturedPiece == 'r') bRookBB |= capBit;
                else if (ui.capturedPiece == 'q') bQueenBB |= capBit;
                else if (ui.capturedPiece == 'k') bKingBB |= capBit;
            }

            if (ui.wasCastle && ui.rookFrom >= 0) {
                U64 rf = 1ULL << ui.rookFrom;
                U64 rt = 1ULL << ui.rookTo;
                if (wRookBB & rt) {
                    wRookBB &= ~rt; wRookBB |= rf;
                }
            }
        }
        else {
            if (bPawnBB & toBit) { bPawnBB &= ~toBit; bPawnBB |= fromBit; }
            else if (bKnightBB & toBit) { bKnightBB &= ~toBit; bKnightBB |= fromBit; }
            else if (bBishopBB & toBit) { bBishopBB &= ~toBit; bBishopBB |= fromBit; }
            else if (bRookBB & toBit) { bRookBB &= ~toBit; bRookBB |= fromBit; }
            else if (bQueenBB & toBit) { bQueenBB &= ~toBit; bQueenBB |= fromBit; }
            else if (bKingBB & toBit) { bKingBB &= ~toBit; bKingBB |= fromBit; }

            if (ui.capturedPiece) {
                U64 capBit = 1ULL << ui.capturedSquare;
                if (ui.capturedPiece == 'P') wPawnBB |= capBit;
                else if (ui.capturedPiece == 'N') wKnightBB |= capBit;
                else if (ui.capturedPiece == 'B') wBishopBB |= capBit;
                else if (ui.capturedPiece == 'R') wRookBB |= capBit;
                else if (ui.capturedPiece == 'Q') wQueenBB |= capBit;
                else if (ui.capturedPiece == 'K') wKingBB |= capBit;
            }

            if (ui.wasCastle && ui.rookFrom >= 0) {
                U64 rf = 1ULL << ui.rookFrom;
                U64 rt = 1ULL << ui.rookTo;
                if (bRookBB & rt) { bRookBB &= ~rt; bRookBB |= rf; }
            }
        }
    }

    // Restore previous en-passant and castling rights
    enPassantSquare = ui.prevEnPassantSquare;
    wKingCastleKRights = ui.prev_wKR; wKingCastleQRights = ui.prev_wQR;
    bKingCastleKRights = ui.prev_bKR; bKingCastleQRights = ui.prev_bQR;

    // Update aggregate bitboards
    allWhiteBB = wPawnBB | wKnightBB | wBishopBB | wRookBB | wQueenBB | wKingBB;
    allBlackBB = bPawnBB | bKnightBB | bBishopBB | bRookBB | bQueenBB | bKingBB;
    allPiecesBB = allWhiteBB | allBlackBB;

    // Flip side back to mover
    turn = !turn;
}