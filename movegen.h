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
