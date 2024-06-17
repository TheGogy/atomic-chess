#pragma once

#ifndef POSITION_H
#define POSITION_H

#include "bitboards.h"

#define FEN_BUFFER_SIZE 100

typedef struct undoinfo {
  U64 entry;
  Piece captured;
  Square enpassant;
} Undoinfo;

typedef struct position {
  U64 pieces[12];         // Occupancy boards for all pieces
  Piece board[64];        // Overall board state
  Color side_to_play;     // Marker for which side moves next
  int ply;                // Number of moves played
  U64 zobrist_hash;       // Current Zobrist hash
  U64 checkers;           // Pieces checking king
  U64 pinned;             // Pinned pieces board

  Undoinfo history[256];  // Game history

} Position;

extern const U64 WHITE_OO_MASK;
extern const U64 WHITE_OOO_MASK;
extern const U64 WHITE_OO_BLOCKERS_MASK;
extern const U64 WHITE_OOO_BLOCKERS_MASK;
extern const U64 WHITE_OOO_IGNORE_DANGER;
extern const U64 BLACK_OO_MASK;
extern const U64 BLACK_OOO_MASK;
extern const U64 BLACK_OO_BLOCKERS_MASK;
extern const U64 BLACK_OOO_BLOCKERS_MASK;
extern const U64 BLACK_OOO_IGNORE_DANGER;
extern const U64 ALL_CASTLING_MASK;
extern const U64 WHITE_EP_RANK;
extern const U64 BLACK_EP_RANK;

extern U64 ZOBRIST_TABLE[6][64];

void init_zobrist_table();

void put_piece(Position *pos, Piece p, Square s);
void remove_piece(Position *pos, Square s);
void move_piece(Position *pos, Square from, Square to);
void move_piece_quiet(Position *pos, Square from, Square to);

void set_from_fen(Position *pos, const char *fen);
void get_fen_from_pos(Position *pos, char *fen);

void print_position(Position *pos);

#endif // !POSITION_H
