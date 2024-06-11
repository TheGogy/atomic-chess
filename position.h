#include "utils.h"
#include "bitboards.h"

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


void init_zobrist_table();

void put_piece(Position *pos, Piece p, Square s);
void remove_piece(Position *pos, Piece p, Square s);
void move_piece(Position *pos, Square from, Square to);
void move_piece_quiet(Position *pos, Square from, Square to);

void set_from_fen(Position *pos, const char *fen);

void print_position(Position *pos);
