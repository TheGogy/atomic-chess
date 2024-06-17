#include "position.h"
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
const U64 WHITE_EP_RANK = 0XFF00000000ULL;
const U64 BLACK_EP_RANK = 0XFF000000ULL;

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

// Adds the specifiec piece to the specified square
inline void put_piece(Position *pos, Piece p, Square s) {
  pos->board[s] = p;
  pos->pieces[p] |= SQUARE_TO_BITBOARD[s];
  pos->zobrist_hash ^= ZOBRIST_TABLE[p][s];
}

// Removes the given piece from the specified Square
// Assumes that the square contains that piece
inline void remove_piece(Position *pos, Square s){
  pos->zobrist_hash ^= ZOBRIST_TABLE[pos->board[s]][s];
  pos->pieces[pos->board[s]] &= ~SQUARE_TO_BITBOARD[s];
  pos->board[s] = NO_PIECE;
}

// Moves whatever piece is on the "from" square to the "to" square
// And vice versa. Used for moves that can be captures.
inline void move_piece(Position *pos, Square from, Square to) {
  pos->zobrist_hash ^= ZOBRIST_TABLE[pos->board[from]][from] ^
    ZOBRIST_TABLE[pos->board[from]][to] ^
    ZOBRIST_TABLE[pos->board[to]][to];

  U64 mask = SQUARE_TO_BITBOARD[from] | SQUARE_TO_BITBOARD[to];
  pos->pieces[pos->board[from]] ^= mask;
  pos->pieces[pos->board[to]] &= ~mask;
  pos->board[to] = pos->board[from];
  pos->board[from] = NO_PIECE;
}

// Moves the piece from the "from" square to the "to" square. Does not
// implement captures.
inline void move_piece_quiet(Position *pos, Square from, Square to) {
  pos->zobrist_hash ^= ZOBRIST_TABLE[pos->board[from]][from] ^
    ZOBRIST_TABLE[pos->board[from]][to];

  pos->pieces[pos->board[from]] ^= (SQUARE_TO_BITBOARD[from] | SQUARE_TO_BITBOARD[to]);
  pos->board[to] = pos->board[from];
  pos->board[from] = NO_PIECE;
}

// Sets the position according to the given FEN.
void set_from_fen(Position *pos, const char *fen) {

  // Clear the board for the position
  for (int i = 0; i < 12; i++) pos->pieces[i] = 0ULL;
  for (int i = 0; i < 64; i++) pos->board[i] = NO_PIECE;

  // Clear position data
  pos->ply = 0;
  pos->history[0].entry = ALL_CASTLING_MASK;
  pos->history[0].enpassant = NO_SQUARE;
  pos->history[0].captured = NO_PIECE;
  pos->zobrist_hash = 0;

  // Pointer to the current character
  const char *fen_ptr = fen;

  // Main part of the FEN (position)

  // Start on last row of bitboard
  int square = a1;

  while (*fen_ptr && *fen_ptr != ' ') {
    char c = *fen_ptr++;
    if (isdigit(c)) {
      square += (c - '0');
    } else if (c == '/') {
      // Go to start of line above
      square -= 16;
    } else {
      put_piece(pos, CHAR_TO_PIECE[c], square++);
    }
  }

  // Go to next part of the fen
  fen_ptr++;

  // Parse side to play
  if (*fen_ptr++ == 'w') {
    pos->side_to_play = WHITE;
  } else {
    pos->side_to_play = BLACK;
  }

  // Go to next part of the fen
  fen_ptr++;

  // Parse castling rights
  while (*fen_ptr && !isspace(*fen_ptr)) {
    switch (*fen_ptr) {
      case 'K':
        pos->history[0].entry &= ~WHITE_OO_MASK;
        break;

      case 'Q':
        pos->history[0].entry &= ~WHITE_OOO_MASK;
        break;

      case 'k':
        pos->history[0].entry &= ~BLACK_OO_MASK;
        break;

      case 'q':
        pos->history[0].entry &= ~BLACK_OOO_MASK;
        break;
    }
    fen_ptr++;
  }

  // Go to next part of the fen
  fen_ptr++;

  if (*fen_ptr != '-') {
    int file = fen_ptr[0] - 'a';
    int rank = fen_ptr[1] - '0' - 1;
    pos->history[0].enpassant = (Square) rank * 8 + file;
  }
}

// Gets the FEN from the position and stores it in the given string.
void get_fen_from_pos(Position *pos, char *fen){
  int index = 0;

  // Main part of the FEN (position)
  int empty_squares = 0;
  for (int rank = 7; rank >= 0; rank--) {
    empty_squares = 0;
    for (int file = 0; file < 8; file++) {
      int square = rank * 8 + file;
      Piece p = pos->board[square];
      if (p == NO_PIECE) empty_squares++;
      else {
        if (empty_squares > 0) {
          index += sprintf(&fen[index], "%d", empty_squares);
        }
        empty_squares = 0;
        index += sprintf(&fen[index], "%c", PIECE_TO_CHAR[p]);
      };
    }
    if (empty_squares > 0) index += sprintf(&fen[index], "%d", empty_squares);
    if (rank > 0) index += sprintf(&fen[index], "/");
  }

  // Side to play
  index += sprintf(&fen[index], " %s", pos->side_to_play == WHITE ? "w " : "b ");

  // Castling rights
  if (!(pos->history[pos->ply].entry & WHITE_OO_MASK   )) index += sprintf(&fen[index], "K");
  if (!(pos->history[pos->ply].entry & WHITE_OOO_MASK  )) index += sprintf(&fen[index], "Q");
  if (!(pos->history[pos->ply].entry & BLACK_OO_MASK   )) index += sprintf(&fen[index], "k");
  if (!(pos->history[pos->ply].entry & BLACK_OOO_MASK  )) index += sprintf(&fen[index], "q");
  if (  pos->history[pos->ply].entry & ALL_CASTLING_MASK) index += sprintf(&fen[index], "- ");

  // En passant square
  Square enpassant_square = pos->history[pos->ply].enpassant;
  sprintf(&fen[index], " %s", (enpassant_square == NO_SQUARE ? "-" : SQUARE_TO_STRING[enpassant_square]));
}

// Prints the position and some other useful info
void print_position(Position *pos) {
  printf("\n");
  for (int rank = 7; rank >= 0; rank--) {
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
  printf("Zobrist hash:       %llu\n", pos->zobrist_hash);
  printf("Ply:                %d\n", pos->ply);
  char fen[FEN_BUFFER_SIZE];
  get_fen_from_pos(pos, fen);
  printf("FEN:                %s\n", fen);
}
