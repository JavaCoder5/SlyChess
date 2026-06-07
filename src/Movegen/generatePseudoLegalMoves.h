#pragma once
#include <src/Constants/Constants.h>
#include <src/Constants/Macros.h>

extern U64 wPawnBB;
extern U64 wKnightBB;
extern U64 wBishopBB;
extern U64 wRookBB;
extern U64 wQueenBB;
extern U64 wKingBB;

extern U64 bPawnBB;
extern U64 bKnightBB;
extern U64 bBishopBB;
extern U64 bRookBB;
extern U64 bQueenBB;
extern U64 bKingBB;

extern U64 allWhiteBB;
extern U64 allBlackBB;
extern U64 allPiecesBB;

extern U64 wPawnAttacks[64];
extern U64 bPawnAttacks[64];
extern U64 wPawnCaptures[64];
extern U64 bPawnCaptures[64];
extern U64 knightAttacks[64];
extern U64 kingAttacks[64];

extern bool turn;
extern int enPassantSquare;

void generatePseudoLegalMoves(Move(*moves)[], bool sideToMove);