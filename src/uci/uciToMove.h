#pragma once
#include <src/Constants/Constants.h>
#include <string>
#include <iostream>

extern U64 wPawnBB;
extern U64 bPawnBB;

extern U64 allPiecesBB;

extern bool turn;

Move UCIToMove(std::string move);