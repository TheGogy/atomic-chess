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
  U64 pieces[2][6];       // Occupancy boards for all pieces
  Piece board[64];        // Overall board state, contains Pieces
  Color side_to_play;     // Marker for which side moves next
  int ply;                // Number of moves played
  U64 zobrist_hash;       // Current Zobrist hash
  Undoinfo history[256];  // Game history
} Position;

// Castling masks
// See position.c for more info
extern const U64 OO_MASK[2];
extern const U64 OOO_MASK[2];
extern const U64 OO_BLOCKERS_MASK[2];
extern const U64 OOO_BLOCKERS_MASK[2];
extern const U64 OOO_IGNORE_DANGER[2];
extern const U64 ALL_CASTLING_MASK;

// En passant ranks
extern const U64 EP_RANK[2];
extern const U64 DOUBLE_PUSH_RANK[2];

extern const U64 NOT_A_FILE;
extern const U64 NOT_H_FILE;
extern const U64 NOT_GH_FILE;
extern const U64 NOT_AB_FILE;

extern U64 ZOBRIST_TABLE[13][64];

void init_zobrist_table();

void put_piece(Position *pos, Piecetype pt, Color col, Square s);
void remove_piece(Position *pos, Square s);
void move_piece(Position *pos, Square from, Square to);
void move_piece_quiet(Position *pos, Square from, Square to);

int set_from_fen(Position *pos, const char *fen);
void get_fen_from_pos(Position *pos, char *fen);

void print_position(Position *pos);

#endif // !POSITION_H
