#include "position.h"
#include "bitboards.h"
#include <ctype.h>
#include <stdio.h>

const U64 OO_MASK[2] = {0x90ULL, 0x9000000000000000ULL};
const U64 OOO_MASK[2] = {0x11ULL, 0x1100000000000000ULL};
const U64 OO_BLOCKERS_MASK[2] = {0x60ULL, 0x6000000000000000ULL};
const U64 OOO_BLOCKERS_MASK[2] = {0xEULL, 0xE00000000000000ULL};
const U64 OOO_IGNORE_DANGER[2] = {0xFFFFFFFFFFFFFFFDULL, 0xFDFFFFFFFFFFFFFFULL};

const U64 DOUBLE_PUSH_RANK[2] = {0xFF00ULL, 0xFF000000000000ULL};
const U64 EP_RANK[2] = {0xFF00000000ULL, 0xFF000000ULL};

const U64 ALL_CASTLING_MASK = 0x9100000000000091ULL;

const U64 NOT_A_FILE = 18374403900871474942ULL;
const U64 NOT_H_FILE = 9187201950435737471ULL;
const U64 NOT_GH_FILE = 4557430888798830399ULL;
const U64 NOT_AB_FILE = 18229723555195321596ULL;

U64 ZOBRIST_TABLE[13][64];

// From stockfish
U64 gen_rand(U64 *s) {
  *s ^= *s >> 12, *s ^= *s >> 25, *s ^= *s >> 27;
  return (U64)*s * 2685821657736338717ULL;
}

void init_zobrist_table() {
  U64 s = 70026072ULL;
  for (int piece = 0; piece < 13; piece++) {
    for (int square = 0; square < 64; square++) {
      ZOBRIST_TABLE[piece][square] = gen_rand(&s);
    }
  }
}

// Adds the specifiec piece to the specified square
inline void put_piece(Position *pos, Piecetype pt, Color col, Square s) {
  pos->board[s] = TYPE_TO_PIECE[col][pt];
  pos->pieces[col][pt] |= SQUARE_TO_BITBOARD[s];
  pos->zobrist_hash ^= ZOBRIST_TABLE[pt][s];
}

// Removes the given piece from the specified Square
inline void remove_piece(Position *pos, Square s){
  pos->zobrist_hash ^= ZOBRIST_TABLE[pos->board[s]][s];
  pos->pieces[PIECE_TO_COLOR[pos->board[s]]][PIECE_TO_TYPE[pos->board[s]]] &= ~SQUARE_TO_BITBOARD[s];
  pos->board[s] = NO_PIECE;
}

// Moves whatever piece is on the "from" square to the "to" square, implementing captures.
inline void move_piece(Position *pos, Square from, Square to) {
  pos->zobrist_hash ^= ZOBRIST_TABLE[pos->board[from]][from] ^
                       ZOBRIST_TABLE[pos->board[from]][to] ^
                       ZOBRIST_TABLE[pos->board[to]][to];

  U64 mask = SQUARE_TO_BITBOARD[from] | SQUARE_TO_BITBOARD[to];
  pos->pieces[PIECE_TO_COLOR[pos->board[from]]][PIECE_TO_TYPE[pos->board[from]]] ^= mask;
  pos->pieces[PIECE_TO_COLOR[pos->board[to]]][PIECE_TO_TYPE[pos->board[to]]] &= ~mask;
  pos->board[to] = pos->board[from];
  pos->board[from] = NO_PIECE;
}

// Moves the piece from the "from" square to the "to" square. Does not
// implement captures.
inline void move_piece_quiet(Position *pos, Square from, Square to) {
  pos->zobrist_hash ^= ZOBRIST_TABLE[pos->board[from]][from] ^
                       ZOBRIST_TABLE[pos->board[from]][to];

  pos->pieces[PIECE_TO_COLOR[pos->board[from]]][PIECE_TO_TYPE[pos->board[from]]] ^= (SQUARE_TO_BITBOARD[from] | SQUARE_TO_BITBOARD[to]);
  pos->board[to] = pos->board[from];
  pos->board[from] = NO_PIECE;
}

// Sets the position according to the given FEN.
void set_from_fen(Position *pos, const char *fen) {

  // Clear the board for the position
  for (int i = 0; i < 12; i++) pos->pieces[i / 6][i % 6] = 0ULL;
  for (int i = 0; i < 64; i++) pos->board[i] = NO_PIECE;

  // Clear position data
  for (int i = 0; i < 256; i++) pos->history[i].entry = ALL_CASTLING_MASK;
  for (int i = 0; i < 256; i++) pos->history[i].enpassant = NO_SQUARE;
  for (int i = 0; i < 256; i++) pos->history[i].captured = NO_PIECE;

  pos->ply = 0;
  pos->zobrist_hash = 0;

  // Pointer to the current character
  const char *fen_ptr = fen;

  // Main part of the FEN (position)

  // Start on last row of bitboard
  int square = a8;

  while (*fen_ptr && *fen_ptr != ' ') {
    char c = *fen_ptr++;
    if (isdigit(c)) {
      square += (c - '0');
    } else if (c == '/') {
      // Go to start of line above
      square -= 16;
    } else {
      Piece p = CHAR_TO_PIECE[c];
      put_piece(pos, PIECE_TO_TYPE[p], PIECE_TO_COLOR[p], square++);
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
        pos->history[0].entry &= ~OO_MASK[WHITE];
        break;

      case 'Q':
        pos->history[0].entry &= ~OOO_MASK[WHITE];
        break;

      case 'k':
        pos->history[0].entry &= ~OO_MASK[BLACK];
        break;

      case 'q':
        pos->history[0].entry &= ~OOO_MASK[BLACK];
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
  if (!(pos->history[pos->ply].entry & OO_MASK[WHITE]   )) index += sprintf(&fen[index], "K");
  if (!(pos->history[pos->ply].entry & OOO_MASK[WHITE]  )) index += sprintf(&fen[index], "Q");
  if (!(pos->history[pos->ply].entry & OO_MASK[BLACK]   )) index += sprintf(&fen[index], "k");
  if (!(pos->history[pos->ply].entry & OOO_MASK[BLACK]  )) index += sprintf(&fen[index], "q");
  if (  pos->history[pos->ply].entry & ALL_CASTLING_MASK ) index += sprintf(&fen[index], "- ");

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
  printf("ply:                %d\n", pos->ply);
  printf("En passant square:  %s\n", SQUARE_TO_STRING[pos->history[pos->ply].enpassant]);
  printf("Castling rights:    %c%c%c%c\n",
         pos->history[pos->ply].entry & OO_MASK[WHITE]  ? '-' : 'K',
         pos->history[pos->ply].entry & OOO_MASK[WHITE] ? '-' : 'Q',
         pos->history[pos->ply].entry & OO_MASK[BLACK]  ? '-' : 'k',
         pos->history[pos->ply].entry & OOO_MASK[BLACK] ? '-' : 'q'
         );
  printf("Zobrist hash:       %llu\n", pos->zobrist_hash);
  printf("Ply:                %d\n", pos->ply);
  char fen[FEN_BUFFER_SIZE];
  get_fen_from_pos(pos, fen);
  printf("FEN:                %s\n", fen);
}
