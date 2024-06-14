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

const U64 WHITE_OO_MASK = 0x90ULL;
const U64 WHITE_OOO_MASK = 0x11ULL;
const U64 WHITE_OO_BLOCKERS_AND_ATTACKERS_MASK = 0x60ULL;
const U64 WHITE_OOO_BLOCKERS_AND_ATTACKERS_MASK = 0xeULL;
const U64 BLACK_OO_MASK = 0x9000000000000000ULL;
const U64 BLACK_OOO_MASK = 0x1100000000000000ULL;
const U64 BLACK_OO_BLOCKERS_AND_ATTACKERS_MASK = 0x6000000000000000ULL;
const U64 BLACK_OOO_BLOCKERS_AND_ATTACKERS_MASK = 0xE00000000000000ULL;
const U64 ALL_CASTLING_MASK = 0x9100000000000091ULL;

extern U64 ZOBRIST_TABLE[6][64];

void init_zobrist_table();

void put_piece(Position *pos, Piece p, Square s);
void remove_piece(Position *pos, Piece p, Square s);
void move_piece(Position *pos, Square from, Square to);
void move_piece_quiet(Position *pos, Square from, Square to);

void set_from_fen(Position *pos, const char *fen);
void get_fen_from_pos(Position *pos, char *fen);

void print_position(Position *pos);
