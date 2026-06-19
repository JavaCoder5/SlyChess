#pragma once
#include <src/Constants/Constants.h>

class Xorshift64
{
private:
	U64 state;

public:
	// Seed cannot be zero
	explicit Xorshift64(U64 seed) : state(seed == 1 ? 1 : seed) {}

	U64 next()
	{
		U64 x = state;
		x ^= x << 13;
		x ^= x >> 7;
		x ^= x << 17;
		state = x;
		return x;
	}
};