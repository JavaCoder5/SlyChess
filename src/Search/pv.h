#pragma once
#include <src/Constants/Constants.h>

constexpr int MAX_PLY = 256;

extern Move pvTable[MAX_PLY][MAX_PLY];
extern int pvLength[MAX_PLY + 1];

inline void resetPVNode(int ply)
{
	if (ply >= 0 && ply <= MAX_PLY)
		pvLength[ply] = ply;
}

inline Move getPVMove(int ply)
{
	if (ply < 0 || ply >= MAX_PLY || pvLength[ply] <= ply)
		return 0;

	return pvTable[ply][ply];
}

inline void updatePV(int ply, Move move)
{
	if (ply < 0 || ply >= MAX_PLY || move == 0)
		return;

	int childLength = pvLength[ply + 1];
	if (childLength < ply + 1 || childLength > MAX_PLY)
		childLength = ply + 1;

	pvTable[ply][ply] = move;
	for (int i = ply + 1; i < childLength; ++i)
		pvTable[ply][i] = pvTable[ply + 1][i];

	pvLength[ply] = childLength;
}
