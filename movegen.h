#pragma once

#ifndef MOVEGEN_H
#define MOVEGEN_H
#include "tables.h"
#include "bitboards.h"
#include "position.h"


typedef enum moveflag {
  QUIET = 0x0,
  DOUBLE_PUSH = 0x1,
  OO = 0x2,
  OOO = 0x3,
  CAPTURE = 0x8,
  EN_PASSANT = 0xA,
  PROMOTIONS = 0x7,
  PROMOTION_CAPTURES = 0xC,
  PR_KNIGHT = 0x4,
  PR_BISHOP = 0x5,
  PR_ROOK = 0x6,
  PR_QUEEN = 0x7,
  PC_KNIGHT = 0xC,
  PC_BISHOP = 0xD,
  PC_ROOK = 0xE,
  PC_QUEEN = 0xF
} MoveFlag;

typedef struct move {
  MoveFlag flags;
  Square from;
  Square to;
} Move;

// Get the bitboard of all diagonal sliders of given color
static inline U64 get_diagonal_sliders(Position *pos, Color c) {
  return c == WHITE ? pos->pieces[WHITE_BISHOP] | pos->pieces[WHITE_QUEEN] :
                      pos->pieces[BLACK_BISHOP] | pos->pieces[BLACK_QUEEN] ;
}

// Get the bitboard of all orthogonal sliders of given color
static inline U64 get_orthogonal_sliders(Position *pos, Color c) {
  return c == WHITE ? pos->pieces[WHITE_ROOK] | pos->pieces[WHITE_QUEEN] :
                      pos->pieces[BLACK_ROOK] | pos->pieces[BLACK_QUEEN] ;
}

// Get the bitboard of all pieces of given color
static inline U64 get_all_pieces(Position *pos, Color c) {
  return c == WHITE ?
    pos->pieces[WHITE_PAWN] | pos->pieces[WHITE_KNIGHT] | pos->pieces[WHITE_BISHOP] |
    pos->pieces[WHITE_ROOK] | pos->pieces[WHITE_QUEEN] | pos->pieces[WHITE_KING]
  :
    pos->pieces[BLACK_PAWN] | pos->pieces[BLACK_KNIGHT] | pos->pieces[BLACK_BISHOP] |
    pos->pieces[BLACK_ROOK] | pos->pieces[BLACK_QUEEN] | pos->pieces[BLACK_KING];
}

// Gets a bitboard of all the pieces of a given color attacking the given square
// inline U64 __attribute__((always_inline)) get_attackers_from(Position *pos, Color c, Square s, U64 occupancies) {
static inline U64 get_attackers_from(Position *pos, Color c, Square s, U64 occupancies) {
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

inline Move* get_moves(Square from, U64 to, Move *list, MoveFlag flags) {
  while (to) {
    Move move;
    move.flags = flags;
    move.from = from;
    move.to = (Square)pop_lsb(&to);
    *list++ = move;
  }
  return list;
}

void play(Position *pos, Color c, Move *m);
void undo(Position *pos, Color c, Move *m);

Move* generate_legal_moves(Position *pos, Move *list);

#endif // !MOVEGEN_H
