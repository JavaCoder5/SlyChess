#include "computeBaseHash.h"

U64 computeBaseHash()
{
	U64 hash = 0;

	// Pieces
	U64 bb = wPawnBB;
	while (bb)
	{
		int sq = count_trailing_zeros(bb);
		hash ^= Zobrist::psq[wPawn][sq];

		U64 bitMask = (1ULL << sq);
		bb = bb & ~bitMask; // Clear the least significant bit
	}

	bb = wKnightBB;
	while (bb)
	{
		int sq = count_trailing_zeros(bb);
		hash ^= Zobrist::psq[wKnight][sq];

		U64 bitMask = (1ULL << sq);
		bb = bb & ~bitMask; // Clear the least significant bit
	}

	bb = wBishopBB;
	while (bb)
	{
		int sq = count_trailing_zeros(bb);
		hash ^= Zobrist::psq[wBishop][sq];

		U64 bitMask = (1ULL << sq);
		bb = bb & ~bitMask; // Clear the least significant bit
	}

	bb = wRookBB;
	while (bb)
	{
		int sq = count_trailing_zeros(bb);
		hash ^= Zobrist::psq[wRook][sq];

		U64 bitMask = (1ULL << sq);
		bb = bb & ~bitMask; // Clear the least significant bit
	}

	bb = wQueenBB;
	while (bb)
	{
		int sq = count_trailing_zeros(bb);
		hash ^= Zobrist::psq[wQueen][sq];

		U64 bitMask = (1ULL << sq);
		bb = bb & ~bitMask; // Clear the least significant bit
	}

	bb = wKingBB;
	while (bb)
	{
		int sq = count_trailing_zeros(bb);
		hash ^= Zobrist::psq[wKing][sq];

		U64 bitMask = (1ULL << sq);
		bb = bb & ~bitMask; // Clear the least significant bit
	}

	bb = bPawnBB;
	while (bb)
	{
		int sq = count_trailing_zeros(bb);
		hash ^= Zobrist::psq[bPawn][sq];

		U64 bitMask = (1ULL << sq);
		bb = bb & ~bitMask; // Clear the least significant bit
	}

	bb = bKnightBB;
	while (bb)
	{
		int sq = count_trailing_zeros(bb);
		hash ^= Zobrist::psq[bKnight][sq];

		U64 bitMask = (1ULL << sq);
		bb = bb & ~bitMask; // Clear the least significant bit
	}

	bb = bBishopBB;
	while (bb)
	{
		int sq = count_trailing_zeros(bb);
		hash ^= Zobrist::psq[bBishop][sq];

		U64 bitMask = (1ULL << sq);
		bb = bb & ~bitMask; // Clear the least significant bit
	}

	bb = bRookBB;
	while (bb)
	{
		int sq = count_trailing_zeros(bb);
		hash ^= Zobrist::psq[bRook][sq];

		U64 bitMask = (1ULL << sq);
		bb = bb & ~bitMask; // Clear the least significant bit
	}

	bb = bQueenBB;
	while (bb)
	{
		int sq = count_trailing_zeros(bb);
		hash ^= Zobrist::psq[bQueen][sq];

		U64 bitMask = (1ULL << sq);
		bb = bb & ~bitMask; // Clear the least significant bit
	}

	bb = bKingBB;
	while (bb)
	{
		int sq = count_trailing_zeros(bb);
		hash ^= Zobrist::psq[bKing][sq];

		U64 bitMask = (1ULL << sq);
		bb = bb & ~bitMask; // Clear the least significant bit
	}

	// castling rights
	int cr = 0;

	if (wKingCastleKRights) cr |= 1 << 0;
	if (wKingCastleQRights) cr |= 1 << 1;
	if (bKingCastleKRights) cr |= 1 << 2;
	if (bKingCastleQRights) cr |= 1 << 3;

	hash ^= Zobrist::castling[cr];

	// en passant
	if (enPassantSquare != -1)
	{
		int file = enPassantSquare & 7;
		hash ^= Zobrist::enpassant[file];
	}

	if (!turn)
	{
		hash ^= Zobrist::sideToMoveKey;
	}

	return hash;
}