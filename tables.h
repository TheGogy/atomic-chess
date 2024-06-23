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

// Initialization functions to populate all lookup tables.
void initialize_all_lookups();

// Get attacks for given piece.
U64 get_rook_attacks(Square square, U64 occupancies);
U64 get_bishop_attacks(Square square, U64 occupancies);
U64 get_queen_attacks(Square square, U64 occupancies);

// Get xray attacks for given piece.
// These will get attacks up to the first blockers, remove the first blockers,
// and return the attacks after that.
U64 get_xray_rook_lookups(Square square, U64 occupancies);
U64 get_xray_bishop_lookups(Square square, U64 occupancies);


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
