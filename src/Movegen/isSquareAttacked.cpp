#include "isSquareAttacked.h"
#include "generatePseudoLegalMoves.h" // for extern bitboards and attack tables
#include <src/Movegen/MagicBoards/lookupAttacks.h>

bool isSquareAttacked(int sq, bool byWhite)
{
	if (sq < 0 || sq >= 64) return false;
	U64 sqBB = 1ULL << sq;

	// Pawn attacks: check potential pawn attacker squares and confirm occupancy
	if (byWhite) {
		int p1 = sq - 7;
		int p2 = sq - 9;
		if (p1 >= 0 && p1 < 64) {
			if ((wPawnBB >> p1) & 1ULL) {
				if (wPawnCaptures[p1] & sqBB) return true;
			}
		}
		if (p2 >= 0 && p2 < 64) {
			if ((wPawnBB >> p2) & 1ULL) {
				if (wPawnCaptures[p2] & sqBB) return true;
			}
		}
	}
	else {
		int p1 = sq + 7;
		int p2 = sq + 9;
		if (p1 >= 0 && p1 < 64) {
			if ((bPawnBB >> p1) & 1ULL) {
				if (bPawnCaptures[p1] & sqBB) return true;
			}
		}
		if (p2 >= 0 && p2 < 64) {
			if ((bPawnBB >> p2) & 1ULL) {
				if (bPawnCaptures[p2] & sqBB) return true;
			}
		}
	}

	// Knights
	if (knightAttacks[sq] & (byWhite ? wKnightBB : bKnightBB)) return true;

	// King
	if (kingAttacks[sq] & (byWhite ? wKingBB : bKingBB)) return true;

	// Sliding pieces: rooks/queens and bishops/queens
	U64 occ = allPiecesBB;
	U64 rookAtt = lookupRookAttacks(sq, occ);
	U64 bishopAtt = lookupBishopAttacks(sq, occ);

	U64 rookAttackers = byWhite ? (wRookBB | wQueenBB) : (bRookBB | bQueenBB);
	if (rookAtt & rookAttackers) return true;

	U64 bishopAttackers = byWhite ? (wBishopBB | wQueenBB) : (bBishopBB | bQueenBB);
	if (bishopAtt & bishopAttackers) return true;

	return false;
}
