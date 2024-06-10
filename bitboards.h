#pragma once
#ifndef BITBOARDS_H
#define BITBOARDS_H

#include "utils.h"

typedef enum color {WHITE, BLACK} Color;
typedef enum piece {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING} Piece;

typedef enum square {
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8,
	NO_SQUARE
} Square;

extern const U64 FILE_MASKS[8];
extern const U64 RANK_MASKS[8];
extern const U64 DIAGONAL_MASKS[15];
extern const U64 ANTI_DIAGONAL_MASKS[15];
extern const U64 SQUARE_TO_BITBOARD[65];
extern const char* sq_to_str[65];

inline Color invert_color(Color c) {
  return (Color)(c ^ BLACK);
}

void print_bitboard(U64 bitboard);

#endif // !BITBOARDS_H
