#pragma once

#ifndef MOVEGEN_H
#define MOVEGEN_H
#include "tables.h"
#include "bitboards.h"
#include "position.h"


typedef enum moveflag {
  QUIET = 0x0,             // 0b0000
  DOUBLE_PUSH = 0x1,       // 0b0001
  OO = 0x2,                // 0b0010
  OOO = 0x3,               // 0b0011
  CAPTURE = 0x8,           // 0b1000
  CAPTURES = 0xF,          // 0b1111
  EN_PASSANT = 0xA,        // 0b1010
  PROMOTIONS = 0x7,        // 0b0111
  PROMOTION_CAPTURES = 0xC,// 0b1100
  PR_KNIGHT = 0x4,         // 0b0100
  PR_BISHOP = 0x5,         // 0b0101
  PR_ROOK = 0x6,           // 0b0110
  PR_QUEEN = 0x7,          // 0b0111
  PC_KNIGHT = 0xC,         // 0b1100
  PC_BISHOP = 0xD,         // 0b1101
  PC_ROOK = 0xE,           // 0b1110
  PC_QUEEN = 0xF           // 0b1111
} MoveFlag;

typedef struct move {
  MoveFlag flags;
  Square from;
  Square to;
} Move;

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

#endif // !MOVEGEN_H
