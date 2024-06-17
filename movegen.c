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
Move* generate_legal_moves(Position *pos, Move *list) {
  Color me = pos->side_to_play;
  Color you = me ^ BLACK;

  U64 my_king_bitboard = pos->pieces[me == WHITE ? WHITE_KING : BLACK_KING];
  U64 your_king_bitboard = pos->pieces[you == WHITE ? WHITE_KING : BLACK_KING];

  Square my_king_square = get_lsb_idx(my_king_bitboard);
  Square your_king_square = get_lsb_idx(your_king_bitboard);

  U64 all_my_pieces = get_all_pieces(pos, me);
  U64 all_your_pieces = get_all_pieces(pos, you);
  U64 your_orthogonal_sliders = get_orthogonal_sliders(pos, you);
  U64 your_diagonal_sliders = get_diagonal_sliders(pos, you);

  U64 all_pieces = all_my_pieces | all_your_pieces;

  U64 orthogonal_pin = 0ULL;
  U64 diagonal_pin = 0ULL;

  U64 attacked = 0ULL; // Squares kin cannot move to
  U64 checkmask = 0ULL; // 1 for all pieces checking king, else all 1s;

  U64 enpassant_target = SQUARE_TO_BITBOARD[pos->history[pos->ply].enpassant];

  // General purpose bitboards used for move generation
  U64 b1, b2, b3;

  // Generate rook pin masks
  if (PSEUDO_LEGAL_ATTACKS[ROOK][my_king_square] & your_orthogonal_sliders) {
    U64 attackHV = get_rook_attacks(my_king_square, all_pieces) & your_orthogonal_sliders;
    U64 pinsHV = get_xray_rook_lookups(my_king_square, all_pieces) & your_orthogonal_sliders;
    while (attackHV) {
      checkmask |= CHECK_BETWEEN[my_king_square][pop_lsb(&attackHV)];
    }
    while (pinsHV) {
      orthogonal_pin |= PIN_BETWEEN[my_king_square][pop_lsb(&pinsHV)];
    }
  }

  // Generate bishop pin masks
  if (PSEUDO_LEGAL_ATTACKS[BISHOP][my_king_square] & your_diagonal_sliders) {
    U64 attackD12 = get_bishop_attacks(my_king_square, all_pieces) & your_diagonal_sliders;
    U64 pinsD12 = get_xray_bishop_lookups(my_king_square, all_pieces) & your_diagonal_sliders;
    while (attackD12) {
      checkmask |= CHECK_BETWEEN[my_king_square][pop_lsb(&attackD12)];
      print_bitboard(checkmask);
    }

    while (pinsD12) {
      diagonal_pin |= PIN_BETWEEN[my_king_square][pop_lsb(&pinsD12)];
    }
  }

  if (!checkmask) checkmask = 0xFFFFFFFFFFFFFFFF;

  // Generate En Passant pin masks
  if (enpassant_target) {
    const U64 my_pawns = pos->pieces[me == WHITE ? WHITE_PAWN : BLACK_PAWN];
    const U64 enpassant_rank = (me == WHITE ? WHITE_EP_RANK : BLACK_EP_RANK);

    if ((enpassant_rank & my_king_bitboard) && (enpassant_rank & your_orthogonal_sliders) && (enpassant_rank & my_pawns)) {
      const U64 enpassant_left = my_pawns & ((enpassant_target & NOT_H_FILE) >> 1);
      const U64 enpassant_right = my_pawns & ((enpassant_target & NOT_A_FILE) << 1);

      if (enpassant_left) {
        const U64 occ_without_ep = all_pieces & ~(enpassant_target | enpassant_left);
        if ((get_rook_attacks(my_king_square, occ_without_ep) & enpassant_rank) & your_orthogonal_sliders) {
          enpassant_target = 0ULL;
        }
      }
      if (enpassant_right) {
        const U64 occ_without_ep = all_pieces & ~(enpassant_target | enpassant_right);
        if ((get_rook_attacks(my_king_square, occ_without_ep) & enpassant_rank) & your_orthogonal_sliders) {
          enpassant_target = 0ULL;
        }
      }
    }
  }

  // Generate attacked squares and add them to the king ban
  attacked |= get_all_knight_attacks(pos->pieces[you == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT]);
  attacked |= get_all_pawn_attacks(pos, you);
  attacked |= KING_ATTACKS[your_king_square];
  b1 = your_orthogonal_sliders;
  b2 = your_diagonal_sliders;
  while (b1) attacked |= get_rook_attacks(pop_lsb(&b1), all_pieces ^ my_king_bitboard);
  while (b2) attacked |= get_bishop_attacks(pop_lsb(&b2), all_pieces ^ my_king_bitboard);

  // King can move to all squares except attacked ones / ones with our pieces
  b1 = KING_ATTACKS[my_king_square] & ~(all_my_pieces | attacked);
  list = get_moves(my_king_square, b1 & ~all_your_pieces, list, QUIET);
  list = get_moves(my_king_square, b1 & all_your_pieces, list, CAPTURE);

  // Generate kingside castling moves
  b1 = me == WHITE ? WHITE_OO_MASK : BLACK_OO_MASK;
  b2 = me == WHITE ? WHITE_OO_BLOCKERS_MASK : BLACK_OO_BLOCKERS_MASK;
  if (!((pos->history[pos->ply].entry & b1) | ((all_pieces | attacked) & b2))) {
    Move m;
    m.flags = OO;
    if (me == WHITE) {
      m.from = e1;
      m.to = h1;
    } else {
      m.from = e8;
      m.to = h8;
    }
    *list++ = m;
  }

  // Generate queenside castling moves
  b1 = me == WHITE ? WHITE_OOO_MASK : BLACK_OOO_MASK;
  b2 = me == WHITE ? WHITE_OOO_BLOCKERS_MASK : BLACK_OOO_BLOCKERS_MASK;
  b3 = me == WHITE ? WHITE_OOO_IGNORE_DANGER : BLACK_OOO_IGNORE_DANGER;
  if (!((pos->history[pos->ply].entry & b1) | ((all_pieces | (attacked & b3)) & b2))) {
    Move m;
    m.flags = OOO;
    if (me == WHITE) {
      m.from = e1;
      m.to = c1;
    } else {
      m.from = e8;
      m.to = c8;
    }
    *list++ = m;
  }

  // Generate knight moves
  // Pinned knights can never move, prune them immediately
  b1 = pos->pieces[me == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT] & ~(orthogonal_pin | diagonal_pin);
  while (b1) {
    Square s = pop_lsb(&b1);
    b2 = KNIGHT_ATTACKS[s] & ~all_my_pieces & checkmask;
    print_bitboard(b2);
    list = get_moves(s, b2 & ~all_your_pieces, list, QUIET);
    list = get_moves(s, b2 & all_your_pieces, list, CAPTURE);
  }

  return list;
}
