#include "bitboards.h"
#include "movegen.h"
#include <stdio.h>

#define U64 unsigned long long

const U64 SQUARE_TO_BITBOARD[65] = {
    0x1ULL, 0x2ULL, 0x4ULL, 0x8ULL,
    0x10ULL, 0x20ULL, 0x40ULL, 0x80ULL,
    0x100ULL, 0x200ULL, 0x400ULL, 0x800ULL,
    0x1000ULL, 0x2000ULL, 0x4000ULL, 0x8000ULL,
    0x10000ULL, 0x20000ULL, 0x40000ULL, 0x80000ULL,
    0x100000ULL, 0x200000ULL, 0x400000ULL, 0x800000ULL,
    0x1000000ULL, 0x2000000ULL, 0x4000000ULL, 0x8000000ULL,
    0x10000000ULL, 0x20000000ULL, 0x40000000ULL, 0x80000000ULL,
    0x100000000ULL, 0x200000000ULL, 0x400000000ULL, 0x800000000ULL,
    0x1000000000ULL, 0x2000000000ULL, 0x4000000000ULL, 0x8000000000ULL,
    0x10000000000ULL, 0x20000000000ULL, 0x40000000000ULL, 0x80000000000ULL,
    0x100000000000ULL, 0x200000000000ULL, 0x400000000000ULL, 0x800000000000ULL,
    0x1000000000000ULL, 0x2000000000000ULL, 0x4000000000000ULL, 0x8000000000000ULL,
    0x10000000000000ULL, 0x20000000000000ULL, 0x40000000000000ULL, 0x80000000000000ULL,
    0x100000000000000ULL, 0x200000000000000ULL, 0x400000000000000ULL, 0x800000000000000ULL,
    0x1000000000000000ULL, 0x2000000000000000ULL, 0x4000000000000000ULL, 0x8000000000000000ULL,
    0x0ULL
};

const char* SQUARE_TO_STRING[65] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "None"
};

const Color PIECE_TO_COLOR[] = {
    [WHITE_PAWN]   = WHITE,
    [WHITE_KNIGHT] = WHITE,
    [WHITE_BISHOP] = WHITE,
    [WHITE_ROOK]   = WHITE,
    [WHITE_QUEEN]  = WHITE,
    [WHITE_KING]   = WHITE,
    [BLACK_PAWN]   = BLACK,
    [BLACK_KNIGHT] = BLACK,
    [BLACK_BISHOP] = BLACK,
    [BLACK_ROOK]   = BLACK,
    [BLACK_QUEEN]  = BLACK,
    [BLACK_KING]   = BLACK,
    [NO_PIECE]     = NO_COLOR
};

const Piecetype PIECE_TO_TYPE[] = {
    [WHITE_PAWN]   = PAWN,
    [WHITE_KNIGHT] = KNIGHT,
    [WHITE_BISHOP] = BISHOP,
    [WHITE_ROOK]   = ROOK,
    [WHITE_QUEEN]  = QUEEN,
    [WHITE_KING]   = KING,
    [BLACK_PAWN]   = PAWN,
    [BLACK_KNIGHT] = KNIGHT,
    [BLACK_BISHOP] = BISHOP,
    [BLACK_ROOK]   = ROOK,
    [BLACK_QUEEN]  = QUEEN,
    [BLACK_KING]   = KING,
    [NO_PIECE]     = NO_TYPE
};

const Piece TYPE_TO_PIECE[2][6] = {
    // WHITE pieces
    {
        WHITE_PAWN,
        WHITE_KNIGHT,
        WHITE_BISHOP,
        WHITE_ROOK,
        WHITE_QUEEN,
        WHITE_KING
    },
    // BLACK pieces
    {
        BLACK_PAWN,
        BLACK_KNIGHT,
        BLACK_BISHOP,
        BLACK_ROOK,
        BLACK_QUEEN,
        BLACK_KING
    }
};

const Piece CHAR_TO_PIECE[] = {
    ['P'] = WHITE_PAWN,
    ['N'] = WHITE_KNIGHT,
    ['B'] = WHITE_BISHOP,
    ['R'] = WHITE_ROOK,
    ['Q'] = WHITE_QUEEN,
    ['K'] = WHITE_KING,

    ['p'] = BLACK_PAWN,
    ['n'] = BLACK_KNIGHT,
    ['b'] = BLACK_BISHOP,
    ['r'] = BLACK_ROOK,
    ['q'] = BLACK_QUEEN,
    ['k'] = BLACK_KING,
};

const char PIECE_TO_CHAR[] = {
    [WHITE_PAWN] = 'P',
    [WHITE_KNIGHT] = 'N',
    [WHITE_BISHOP] = 'B',
    [WHITE_ROOK] = 'R',
    [WHITE_QUEEN] = 'Q',
    [WHITE_KING] = 'K',

    [BLACK_PAWN] = 'p',
    [BLACK_KNIGHT] = 'n',
    [BLACK_BISHOP] = 'b',
    [BLACK_ROOK] = 'r',
    [BLACK_QUEEN] = 'q',
    [BLACK_KING] = 'k',

    [NO_PIECE] = '.',
};

const char *MOVETYPE_TO_STR[] = {
    [QUIET] = "QUIET",
    [DOUBLE_PUSH] = "DOUBLE_PUSH",
    [OO] = "OO",
    [OOO] = "OOO",
    [CAPTURE] = "CAPTURE",
    [EN_PASSANT] = "EN_PASSANT",
    [PR_KNIGHT] = "PR_KNIGHT",
    [PR_BISHOP] = "PR_BISHOP",
    [PR_ROOK] = "PR_ROOK",
    [PR_QUEEN] = "PR_QUEEN",
    [PC_KNIGHT] = "PC_KNIGHT",
    [PC_BISHOP] = "PC_BISHOP",
    [PC_ROOK] = "PC_ROOK",
    [PC_QUEEN] = "PC_QUEEN"
};

void print_bitboard(U64 bitboard) {
    printf("\n");
    for (int rank = 7; rank >= 0; rank--) {
        printf(" %d ", rank + 1);
        for (int file = 0 ; file < 8; file++) {
            int square = rank * 8 + file;
            printf(" %s", get_bit(bitboard, square) ? "#" : ".");
        }
        printf("\n");
    }
    printf("\n    a b c d e f g h\n\n");
    printf("Bitboard: %llud\n", bitboard);
}


