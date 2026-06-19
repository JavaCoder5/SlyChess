#pragma once
#include <src/Constants/Constants.h>
#include <src/Constants/Macros.h>
#include <src/zobrist/zobrist.h>
#include <iostream>

extern U64 wPawnBB, wKnightBB, wBishopBB, wRookBB, wQueenBB, wKingBB;

extern U64 bPawnBB, bKnightBB, bBishopBB, bRookBB, bQueenBB, bKingBB;

extern bool wKingCastleKRights;
extern bool wKingCastleQRights;

extern bool bKingCastleKRights;
extern bool bKingCastleQRights;

extern int enPassantSquare;

extern bool turn;

U64 computeBaseHash();