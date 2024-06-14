#include "movegen.h"
#include "bitboards.h"
#include "position.h"

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
