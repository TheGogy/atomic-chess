#include "position.h"
#include "bitboards.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>

const U64 WHITE_OO_MASK = 0x90ULL;
const U64 WHITE_OOO_MASK = 0x11ULL;
const U64 WHITE_OO_BLOCKERS_AND_ATTACKERS_MASK = 0x60ULL;
const U64 WHITE_OOO_BLOCKERS_AND_ATTACKERS_MASK = 0xeULL;
const U64 BLACK_OO_MASK = 0x9000000000000000ULL;
const U64 BLACK_OOO_MASK = 0x1100000000000000ULL;
const U64 BLACK_OO_BLOCKERS_AND_ATTACKERS_MASK = 0x6000000000000000ULL;
const U64 BLACK_OOO_BLOCKERS_AND_ATTACKERS_MASK = 0xE00000000000000ULL;
const U64 ALL_CASTLING_MASK = 0x9100000000000091ULL;

U64 ZOBRIST_TABLE[6][64];

// From stockfish
U64 gen_rand(U64 *s) {
  *s ^= *s >> 12, *s ^= *s >> 25, *s ^= *s >> 27;
  return (U64)*s * 2685821657736338717ULL;
}

void init_zobrist_table() {
  U64 s = 70026072ULL;
  for (int piece = 0; piece < 6; piece++) {
    for (int square = 0; square < 64; square++) {
      ZOBRIST_TABLE[piece][square] = gen_rand(&s);
    }
  }
}

void put_piece(Position *pos, Piece p, Square s) {
  pos->board[s] = p;
  pos->pieces[p] |= SQUARE_TO_BITBOARD[s];
  pos->zobrist_hash ^= ZOBRIST_TABLE[p][s];
}

void remove_piece(Position *pos, Piece p, Square s){
  pos->zobrist_hash ^= ZOBRIST_TABLE[p][s];
  pos->pieces[pos->board[s]] &= ~SQUARE_TO_BITBOARD[s];
  pos->board[s] = NO_PIECE;
}

void move_piece(Position *pos, Square from, Square to) {
  pos->zobrist_hash ^= ZOBRIST_TABLE[pos->board[from]][from] ^
                       ZOBRIST_TABLE[pos->board[from]][to] ^
                       ZOBRIST_TABLE[pos->board[to]][to];

  U64 mask = SQUARE_TO_BITBOARD[from] | SQUARE_TO_BITBOARD[to];
  pos->pieces[pos->board[from]] ^= mask;
  pos->pieces[pos->board[to]] &= ~mask;
  pos->board[to] = pos->board[from];
  pos->board[from] = NO_PIECE;
}

void move_piece_quiet(Position *pos, Square from, Square to) {
  pos->zobrist_hash ^= ZOBRIST_TABLE[pos->board[from]][from] ^
                       ZOBRIST_TABLE[pos->board[from]][to];

  pos->pieces[pos->board[from]] ^= (SQUARE_TO_BITBOARD[from] | SQUARE_TO_BITBOARD[to]);
  pos->board[to] = pos->board[from];
  pos->board[from] = NO_PIECE;
}

void print_position(Position *pos) {
  printf("\n");
  for (int rank = 0; rank < 8; rank++) {
    printf(" %d ", rank + 1);
    for (int file = 0; file < 8 ; file++) {
      int square = rank * 8 + file;
      printf(" %c", PIECE_TO_CHAR[pos->board[square]]);
    }
    printf("\n");
  }
  printf("\n    a b c d e f g h\n\n");
  printf("Side to move:       %s\n", pos->side_to_play ? "black" : "white");
  printf("En passant square:  %s\n", SQUARE_TO_STRING[pos->history[pos->ply].enpassant]);
  printf("Castling rights:    %c%c%c%c\n",
         pos->history[pos->ply].entry & WHITE_OO_MASK  ? '-' : 'K',
         pos->history[pos->ply].entry & WHITE_OOO_MASK ? '-' : 'Q',
         pos->history[pos->ply].entry & BLACK_OO_MASK  ? '-' : 'k',
         pos->history[pos->ply].entry & BLACK_OOO_MASK ? '-' : 'q'
         );
}

void set_from_fen(Position *pos, const char *fen) {
  int square = a8;

  const char *fen_ptr = fen;
  while (*fen_ptr && *fen_ptr != ' ') {
    char c = *fen_ptr++;
  
    if (isdigit(c)) {
      square += (c - '0');
    } else {
      put_piece(pos, CHAR_TO_PIECE[*fen], *fen_ptr);
    }
  }

}
