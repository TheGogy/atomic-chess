#pragma once

#include "position.h"
#ifndef TABLES_H
#define TABLES_H

#include "bitboards.h"

// For these pieces, it is faster to just use a lookup table.
// Ensure that the initialization functions are called before
// using these.
extern const U64 KING_ATTACKS[64];
extern const U64 KNIGHT_ATTACKS[64];
extern const U64 WHITE_PAWN_ATTACKS[64];
extern const U64 BLACK_PAWN_ATTACKS[64];

// Rook and bishop masks, assuming no blockers
extern U64 ROOK_MASKS[64];
extern U64 BISHOP_MASKS[64];

// Bitboard of all squares between two pieces (not including squares themselves)
extern U64 SQUARES_BETWEEN[64][64];

// Bitboard of all squares along the line between two pieces.
extern U64 LINE_BETWEEN[64][64];

// Path from enemy to king including square behind king (excluding enemy)
// [king_square][enemy_square]
extern U64 CHECK_BETWEEN[64][64];

// Path between king and enemy. Zero if not slider
// [king_square][enemy_square]
extern U64 PIN_BETWEEN[64][64];

// Initialization functions to populate all lookup tables
void initialize_rook_attacks();
void initialize_bishop_attacks();
void initialize_all_lookups();

U64 mask_bishop_attacks_otf(Square s, U64 occupancies);

// Get attacks for given piece.
U64 get_rook_attacks(Square square, U64 occupancies);
U64 get_xray_rook_lookups(Square square, U64 occupancies);

U64 get_bishop_attacks(Square square, U64 occupancies);
U64 get_xray_bishop_lookups(Square square, U64 occupancies);

U64 get_queen_attacks(Square square, U64 occupancies);

// All squares that aren't on the A file
extern const U64 NOT_A_FILE;
extern const U64 NOT_H_FILE;
extern const U64 NOT_GH_FILE;
extern const U64 NOT_AB_FILE;

// Get all pawn attacks at once for given color.
inline U64 get_all_pawn_attacks(Position *pos, Color c) {
  U64 pawn_bb = pos->pieces[c == WHITE ? WHITE_PAWN : BLACK_PAWN];
  return c == WHITE ?
    (pawn_bb & NOT_H_FILE) << 7 |
    (pawn_bb & NOT_A_FILE) << 9
  :
    (pawn_bb & NOT_H_FILE) >> 7 |
    (pawn_bb & NOT_A_FILE) >> 9;
}

// Get all knight attacks at once
inline U64 get_all_knight_attacks(U64 knight_bb) {
  U64 l1 = (knight_bb >> 1) & NOT_H_FILE;
  U64 l2 = (knight_bb >> 2) & NOT_GH_FILE;
  U64 r1 = (knight_bb << 1) & NOT_A_FILE;
  U64 r2 = (knight_bb << 2) & NOT_AB_FILE;
  U64 h1 = l1 | r1;
  U64 h2 = l2 | r2;
  return (h1<<16) | (h1>>16) | (h2<<8) | (h2>>8);
}

#endif // !TABLES_H
