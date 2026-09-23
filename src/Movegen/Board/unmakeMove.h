#pragma once
#include <src/Constants/Constants.h>
#include <src/zobrist/zobrist.h>

extern U64 wPawnBB, wKnightBB, wBishopBB, wRookBB, wQueenBB, wKingBB;

extern U64 bPawnBB, bKnightBB, bBishopBB, bRookBB, bQueenBB, bKingBB;

extern U64 allWhiteBB, allBlackBB, allPiecesBB;

extern UndoInfo history[MAX_UNDO];
extern int historyTop;

extern int enPassantSquare;

extern bool turn;

extern bool wKingCastleKRights, wKingCastleQRights;

extern bool bKingCastleKRights, bKingCastleQRights;

extern U64 boardHash;

void unmakeMove(Move m);