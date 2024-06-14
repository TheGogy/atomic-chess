#pragma once

#ifndef MOVEGEN_H
#define MOVEGEN_H
#include "tables.h"
#include "bitboards.h"
#include "position.h"

// Get the bitboard of all diagonal sliders of given color
inline U64 get_diagonal_sliders(Position *pos, Color c) {
  return c == WHITE ? pos->pieces[WHITE_BISHOP] | pos->pieces[WHITE_QUEEN] :
                      pos->pieces[BLACK_BISHOP] | pos->pieces[BLACK_QUEEN] ;
}

// Get the bitboard of all orthogonal sliders of given color
inline U64 get_orthogoal_sliders(Position *pos, Color c) {
  return c == WHITE ? pos->pieces[WHITE_ROOK] | pos->pieces[WHITE_QUEEN] :
                      pos->pieces[BLACK_ROOK] | pos->pieces[BLACK_QUEEN] ;
}

// Get the bitboard of all pieces of given color
inline U64 get_all_pieces(Position *pos, Color c) {
  return c == WHITE ?
    pos->pieces[WHITE_PAWN] | pos->pieces[WHITE_KNIGHT] | pos->pieces[WHITE_BISHOP] |
    pos->pieces[WHITE_ROOK] | pos->pieces[WHITE_QUEEN] | pos->pieces[WHITE_KING]
  :
    pos->pieces[BLACK_PAWN] | pos->pieces[BLACK_KNIGHT] | pos->pieces[BLACK_BISHOP] |
    pos->pieces[BLACK_ROOK] | pos->pieces[BLACK_QUEEN] | pos->pieces[BLACK_KING];
}

// Gets a bitboard of all the pieces of a given color attacking the given square
// inline U64 __attribute__((always_inline)) get_attackers_from(Position *pos, Color c, Square s, U64 occupancies) {
inline U64 get_attackers_from(Position *pos, Color c, Square s, U64 occupancies) {
  return c == WHITE ?
    (BLACK_PAWN_ATTACKS[s] & pos->pieces[WHITE_PAWN]) |
    (KNIGHT_ATTACKS[s] & pos->pieces[WHITE_KNIGHT])   |
    (get_bishop_attacks(s, occupancies) & (pos->pieces[WHITE_BISHOP] | pos->pieces[WHITE_QUEEN])) |
    (get_rook_attacks(s, occupancies) & (pos->pieces[WHITE_ROOK] | pos->pieces[WHITE_QUEEN]))
  :
    (WHITE_PAWN_ATTACKS[s] & pos->pieces[BLACK_PAWN]) |
    (KNIGHT_ATTACKS[s] & pos->pieces[BLACK_KNIGHT])   |
    (get_bishop_attacks(s, occupancies) & (pos->pieces[BLACK_BISHOP] | pos->pieces[BLACK_QUEEN])) |
    (get_rook_attacks(s, occupancies) & (pos->pieces[BLACK_ROOK] | pos->pieces[BLACK_QUEEN]));
}

#endif // !MOVEGEN_H
