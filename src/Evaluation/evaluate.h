#pragma once
#include <src/Constants/Constants.h>
#include <src/Constants/Macros.h>
#include <iostream>

extern U64 wPawnBB, wKnightBB, wBishopBB, wRookBB, wQueenBB, wKingBB;

extern U64 bPawnBB, bKnightBB, bBishopBB, bRookBB, bQueenBB, bKingBB;

extern U64 allPiecesBB;

extern bool turn;

inline int invertPST(const int (*pst)[64], int index);

int evaluate();