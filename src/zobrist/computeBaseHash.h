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