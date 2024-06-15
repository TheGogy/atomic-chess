#include "movegen.h"
#include "bitboards.h"
#include "position.h"
#include "tables.h"

// Plays a move in the given position
void play(Position *pos, Color c, Move *m){
  // Switch side to play
  pos->side_to_play ^= BLACK;

  // Increment game ply
  pos->ply++;

  // Copy move history
  pos->history[pos->ply] = pos->history[pos->ply - 1];

  // Update move history
  pos->history[pos->ply].entry |= SQUARE_TO_BITBOARD[m->to] | SQUARE_TO_BITBOARD[m->from];

  // Perform move based on move type
  switch (m->flags) {
    case QUIET:
      // Move without taking any pieces
      move_piece_quiet(pos, m->from, m->to);
      break;
      
    case DOUBLE_PUSH:
      // Double pawn push
      move_piece_quiet(pos, m->from, m->to);
      // Update en passant square
      pos->history[pos->ply].enpassant = m->from + (c == WHITE ? 8 : -8);
      break;

    case OO:
      // Castling kingside
      if (c == WHITE) {
        move_piece_quiet(pos, e1, g1);
        move_piece_quiet(pos, h1, f1);
      } else {
        move_piece_quiet(pos, e8, g8);
        move_piece_quiet(pos, h8, f8);
      }
      break;

    case OOO:
      // Castling queenside
      if (c == WHITE) {
        move_piece_quiet(pos, e1, c1);
        move_piece_quiet(pos, a1, d1);
      } else {
        move_piece_quiet(pos, e8, c8);
        move_piece_quiet(pos, a8, d8);
      }
      break;

    case EN_PASSANT:
      // Taking en passant
      move_piece_quiet(pos, m->from, m->to);
      remove_piece(pos, (c == WHITE ? -8 : 8));
      break;

    case PR_KNIGHT:
      // Promoting to a knight
      remove_piece(pos, m->from);
      put_piece(pos, (c == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT), m->to);
      break;

    case PR_BISHOP:
      // Promoting to a bishop
      remove_piece(pos, m->from);
      put_piece(pos, (c == WHITE ? WHITE_BISHOP : BLACK_BISHOP), m->to);
      break;

    case PR_ROOK:
      // Promoting to a rook
      remove_piece(pos, m->from);
      put_piece(pos, (c == WHITE ? WHITE_ROOK : BLACK_ROOK), m->to);
      break;

    case PR_QUEEN:
      // Promoting to a queen
      remove_piece(pos, m->from);
      put_piece(pos, (c == WHITE ? WHITE_QUEEN : BLACK_QUEEN), m->to);
      break;

    case PC_KNIGHT:
      // Promoting to a knight + capturing piece
      remove_piece(pos, m->from);
      pos->history[pos->ply].captured = pos->board[m->to];
      remove_piece(pos, m->to);
      put_piece(pos, (c == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT), m->to);
      break;

    case PC_BISHOP:
      // Promoting to a bishop + capturing piece
      remove_piece(pos, m->from);
      pos->history[pos->ply].captured = pos->board[m->to];
      remove_piece(pos, m->to);
      put_piece(pos, (c == WHITE ? WHITE_BISHOP : BLACK_BISHOP), m->to);
      break;

    case PC_ROOK:
      // Promoting to a rook + capturing piece
      remove_piece(pos, m->from);
      pos->history[pos->ply].captured = pos->board[m->to];
      remove_piece(pos, m->to);
      put_piece(pos, (c == WHITE ? WHITE_ROOK : BLACK_ROOK), m->to);
      break;

    case PC_QUEEN:
      // Promoting to a queen + capturing piece
      remove_piece(pos, m->from);
      pos->history[pos->ply].captured = pos->board[m->to];
      remove_piece(pos, m->to);
      put_piece(pos, (c == WHITE ? WHITE_QUEEN : BLACK_QUEEN), m->to);
      break;

    case CAPTURE:
      // Capturing piece
      pos->history[pos->ply].captured = pos->board[m->to];
      move_piece(pos, m->from, m->to);
      break;
  }
}

// Undoes the given move in the given position
void undo(Position *pos, Color c, Move *m){
  switch (m->flags) {
    case QUIET:
      // Move without taking any pieces
      move_piece_quiet(pos, m->to, m->from);
      break;

    case DOUBLE_PUSH:
      // Double pawn push
      move_piece_quiet(pos, m->to, m->from);
      break;

    case OO:
      // Castling kingside
      if (c == WHITE) {
        move_piece_quiet(pos, g1, e1);
        move_piece_quiet(pos, f1, h1);
      } else {
        move_piece_quiet(pos, g8, e8);
        move_piece_quiet(pos, h8, f8);
      }
      break;

    case OOO:
      // Castling queenside
      if (c == WHITE) {
        move_piece_quiet(pos, c1, e1);
        move_piece_quiet(pos, d1, a1);
      } else {
        move_piece_quiet(pos, c8, e8);
        move_piece_quiet(pos, d8, a8);
      }
      break;

    case EN_PASSANT:
      // Capturing en passant
      move_piece_quiet(pos, m->to, m->from);
      put_piece(pos, (c == WHITE ? BLACK_PAWN : WHITE_PAWN), (c == WHITE ? -8 : 8));
      break;

    // Promoting to the given piece
    case PR_KNIGHT:
    case PR_BISHOP:
    case PR_ROOK:
    case PR_QUEEN:
      remove_piece(pos, m->to);
      put_piece(pos, (c == WHITE? WHITE_PAWN : BLACK_PAWN), m->from);
      break;

    // Promoting to the given piece through a capture
    case PC_KNIGHT:
    case PC_BISHOP:
    case PC_ROOK:
    case PC_QUEEN:
      remove_piece(pos, m->to);
      put_piece(pos, (c == WHITE? WHITE_PAWN : BLACK_PAWN), m->from);
      put_piece(pos, pos->history[pos->ply].captured, m->to);
      break;

    case CAPTURE:
      // Capturing piece
      move_piece_quiet(pos, m->to, m->from);
      put_piece(pos, pos->history[pos->ply].captured, m->to);
      break;
  }
  pos->side_to_play ^= BLACK;
  --pos->ply;
}

// Gets the rook + queen pins
inline U64 get_pinhv(Position *pos, U64 your_orthogonal_sliders, Square my_king) {
  U64 pinHV = 0ULL;
  // TODO: Check if this works without making a new variable
  U64 targets = your_orthogonal_sliders;
  Square enemy_piece;
  while (targets) {
    enemy_piece = pop_lsb(&targets);
    pinHV |= PIN_BETWEEN[my_king][enemy_piece]; 
  }
  return pinHV;
}

// Gets the bishop + queen pins
inline U64 get_pind12(Position *pos, U64 your_diagonal_sliders, Square my_king, U64 *ep_target) {
  U64 pinD12 = 0ULL;
  // TODO: Check if this works without making a new variable
  U64 targets = your_diagonal_sliders;
  Square enemy_piece;
  while (targets) {
    enemy_piece = pop_lsb(&targets);
    pinD12 |= PIN_BETWEEN[my_king][enemy_piece];

    // Have to account for en passant
    // https://lichess.org/editor?fen=6q1%2F8%2F8%2F3pP3%2F8%2F1K6%2F8%2F8+w+-+-+0+1
    if (PIN_BETWEEN[my_king][enemy_piece] & *ep_target) *ep_target = 0ULL;
  }
  return pinD12;
}

// Generates all legal moves for the given position and increments pointer to
// last move in move list
Move* generate_legal_moves(Position *pos, Move *list) {
  Color me = pos->side_to_play;
  Color you = me ^ BLACK;

  U64 my_king = pos->pieces[me == WHITE ? WHITE_KING : BLACK_KING];
  U64 your_king = pos->pieces[you == WHITE ? WHITE_KING : BLACK_KING];

  Square my_king_square = get_lsb_idx(my_king);
  Square your_king_square = get_lsb_idx(your_king);

  U64 all_my_pieces = get_all_pieces(pos, me);
  U64 all_your_pieces = get_all_pieces(pos, you);
  U64 my_orthogonal_sliders = get_orthogonal_sliders(pos, me);
  U64 your_orthogonal_sliders = get_orthogonal_sliders(pos, you);
  U64 my_diagonal_sliders = get_diagonal_sliders(pos, me);
  U64 your_diagonal_sliders = get_diagonal_sliders(pos, you);

  U64 all_pieces = all_my_pieces | all_your_pieces;

  U64 rook_pin = 0ULL;
  U64 bishop_pin = 0ULL;

  print_bitboard(all_pieces);

  // if (ROOK_MASKS[my_king_square] & your_orthogonal_sliders) {
  //   U64 attackHV = get_rook_attacks(my_king, all_pieces) & your_orthogonal_sliders;
  //   while (attackHV) {
  //     
  //   }
  // }
  return list;
}
