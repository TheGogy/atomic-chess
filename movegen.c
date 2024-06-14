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

// Generates all legal moves for the given position and increments pointer to
// last move in move list
Move* generate_legal_moves(Position *pos, Color me, Move *list) {
  Color you = me ^ BLACK;
  const U64 my_pieces = get_all_pieces(pos, me);
  const U64 your_pieces = get_all_pieces(pos, you);
  const U64 all_pieces = my_pieces | your_pieces;

  const Square my_king = get_lsb_idx(pos->pieces[me == WHITE ? WHITE_KING : BLACK_KING]);
  const Square your_king = get_lsb_idx(pos->pieces[you == WHITE ? WHITE_KING : BLACK_KING]);

  const U64 my_diagonal_sliders = get_diagonal_sliders(pos, me);
  const U64 your_diagonal_sliders = get_diagonal_sliders(pos,you);
  const U64 my_orthogonal_sliders = get_orthogoal_sliders(pos, me);
  const U64 your_orthogonal_sliders = get_orthogoal_sliders(pos, you);

  U64 b1, b2, b3;

  U64 danger_squares = 0ULL;

  // Add leaping piece attacks
  danger_squares |= get_all_pawn_attacks(pos->pieces[you == WHITE ? WHITE_PAWN : BLACK_PAWN], you);
  danger_squares |= get_all_knight_attacks(pos->pieces[you == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT]);

  // Add diagonal sliders attacks
  b1 = your_diagonal_sliders;
  b2 = your_orthogonal_sliders;

  while (b1) danger_squares |= get_bishop_attacks(pop_lsb(&b1), all_pieces ^ SQUARE_TO_BITBOARD[my_king]);
  while (b2) danger_squares |= get_rook_attacks(pop_lsb(&b2), all_pieces ^ SQUARE_TO_BITBOARD[my_king]);

  // Add king attacks
  danger_squares |= KING_ATTACKS[your_king];

  // Add king moves
  b1 = KING_ATTACKS[my_king] & ~(my_pieces | danger_squares);
  list = get_moves(my_king, b1 & ~your_pieces, list, QUIET);
  list = get_moves(my_king, b1 & your_pieces, list, CAPTURE);


  return list;
}
