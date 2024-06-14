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
      // We just need to move the pieces
      move_piece_quiet(pos, m->from, m->to);
      break;
      
    case DOUBLE_PUSH:
      // Move the piece
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
      remove_piece(pos, m->from);
      put_piece(pos, (c == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT), m->to);
      break;

    case PR_BISHOP:
      remove_piece(pos, m->from);
      put_piece(pos, (c == WHITE ? WHITE_BISHOP : BLACK_BISHOP), m->to);
      break;

    case PR_ROOK:
      remove_piece(pos, m->from);
      put_piece(pos, (c == WHITE ? WHITE_ROOK : BLACK_ROOK), m->to);
      break;

    case PR_QUEEN:
      remove_piece(pos, m->from);
      put_piece(pos, (c == WHITE ? WHITE_QUEEN : BLACK_QUEEN), m->to);
      break;

    case PC_KNIGHT:
      remove_piece(pos, m->from);
      pos->history[pos->ply].captured = pos->board[m->to];
      remove_piece(pos, m->to);
      put_piece(pos, (c == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT), m->to);
      break;

    case PC_BISHOP:
      remove_piece(pos, m->from);
      pos->history[pos->ply].captured = pos->board[m->to];
      remove_piece(pos, m->to);
      put_piece(pos, (c == WHITE ? WHITE_BISHOP : BLACK_BISHOP), m->to);
      break;

    case PC_ROOK:
      remove_piece(pos, m->from);
      pos->history[pos->ply].captured = pos->board[m->to];
      remove_piece(pos, m->to);
      put_piece(pos, (c == WHITE ? WHITE_ROOK : BLACK_ROOK), m->to);
      break;

    case PC_QUEEN:
      remove_piece(pos, m->from);
      pos->history[pos->ply].captured = pos->board[m->to];
      remove_piece(pos, m->to);
      put_piece(pos, (c == WHITE ? WHITE_QUEEN : BLACK_QUEEN), m->to);
      break;


    case CAPTURE:
      pos->history[pos->ply].captured = pos->board[m->to];
      move_piece(pos, m->from, m->to);
      break;
  }
}

