#pragma once
#ifndef BITBOARDS_H
#define BITBOARDS_H

#define U64 unsigned long long

// Bitboard utilities
#define get_bit(bitboard, square) ((bitboard) &   (1ULL << (square)))
#define set_bit(bitboard, square) ((bitboard) |=  (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

#define count_bits(bitboard) __builtin_popcountll(bitboard)
#define get_lsb_idx(bitboard) __builtin_ctzll(bitboard)

// Minimize branching
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

inline int pop_lsb(U64 *bitboard) {
    int lsb_idx = get_lsb_idx(*bitboard);
    *bitboard &= *bitboard - 1; // Pop lsb
    return lsb_idx;
}

typedef enum color {WHITE, BLACK, NO_COLOR} Color;

typedef enum piecetype {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NO_TYPE} Piecetype;

typedef enum piece {
    NO_PIECE,
    WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING
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
extern const Color PIECE_TO_COLOR[];
extern const Piecetype PIECE_TO_TYPE[];
extern const Piece TYPE_TO_PIECE[2][6];
extern const Piece CHAR_TO_PIECE[];
extern const char PIECE_TO_CHAR[];
extern const char *MOVETYPE_TO_STR[];

inline Color invert_color(Color c) {
    return (Color)(c ^ BLACK);
}

inline Square string_to_square(char file, char rank) {
    return (Square)((rank - '1') * 8 + (file - 'a'));
}

void print_bitboard(U64 bitboard);

#endif // !BITBOARDS_H
