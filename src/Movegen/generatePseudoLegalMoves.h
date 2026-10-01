/*
    Copyright (C) 2026 JavaCoder5

    SlyChess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SlyChess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <src/Constants/Constants.h>
#include <src/Constants/Macros.h>
#include <src/Movegen/MagicBoards/lookupAttacks.h>

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

extern bool wKingCastleKRights;
extern bool wKingCastleQRights;

extern bool bKingCastleKRights;
extern bool bKingCastleQRights;

void generatePseudoLegalMoves(Move(*moves)[], bool sideToMove, int *size);