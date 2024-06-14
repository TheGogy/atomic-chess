#pragma once
#ifndef BITBOARDS_H
#define BITBOARDS_H

#define U64 unsigned long long

// Bitboard utilities
#define get_bit(bitboard, square) ((bitboard) &   (1ULL << (square)))
#define set_bit(bitboard, square) ((bitboard) |=  (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

#define count_bits(bitboard) __builtin_popcountll(bitboard)
#define get_lsb_idx(bitboard) (bitboard ? __builtin_ctzll(bitboard) : -1)

typedef enum color {WHITE, BLACK} Color;
typedef enum piecetype {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING} Piecetype;
typedef enum piece {
  WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
  BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
  NO_PIECE
} Piece;


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

extern const char* SQUARE_TO_STRING[65];
extern const Piece CHAR_TO_PIECE[];
extern const char PIECE_TO_CHAR[];

inline Color invert_color(Color c) {
  return (Color)(c ^ BLACK);
}

void print_bitboard(U64 bitboard);

#endif // !BITBOARDS_H
