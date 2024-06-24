#pragma once

#ifndef TABLES_H
#define TABLES_H
#include "position.h"
#include "bitboards.h"

// Piece attack lookups. Assumes empty board.
// Ensure that the initialization functions are called before
// using these.
extern const U64 KING_ATTACKS[64];
extern const U64 KNIGHT_ATTACKS[64];
extern const U64 PAWN_ATTACKS[2][64];
extern const U64 BISHOP_ATTACKS[64];
extern const U64 ROOK_ATTACKS[64];

// Pin between king and enemy. Zero if no straight path.
// [king_square][enemy_square]
extern U64 PIN_BETWEEN[64][64];

void init_pin_between();

// Get all the squares attacked by the pawns of the given color.
static inline U64 get_all_pawn_attacks(U64 pawn_bb, Color c) {
  return c == WHITE ?
    (pawn_bb & NOT_H_FILE) << 9 |
    (pawn_bb & NOT_A_FILE) << 7
  :
    (pawn_bb & NOT_H_FILE) >> 7 |
    (pawn_bb & NOT_A_FILE) >> 9;
}

#endif // !TABLES_H
