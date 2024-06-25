#include "movegen.h"
#include "bitboards.h"
#include "position.h"
#include "tables.h"
#include "sliders.h"
#include <stdio.h>
#include <string.h>

// Plays a move in the given position
void play(Position *pos, Move *m){
  Color c = pos->side_to_play;

  // Switch side to play
  pos->side_to_play ^= BLACK;

  // Increment game ply
  pos->ply++;

  // Copy move history
  pos->history[pos->ply].entry = pos->history[pos->ply - 1].entry;

  // Update move history
  pos->history[pos->ply].entry |= SQUARE_TO_BITBOARD[m->to] | SQUARE_TO_BITBOARD[m->from];
  pos->history[pos->ply].captured = NO_PIECE;
  pos->history[pos->ply].enpassant = NO_SQUARE;

  // Perform move based on move type
  switch (m->flag) {
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
      remove_piece(pos, m->to + (c == WHITE ? -8 : 8));
      break;

    case PR_KNIGHT:
      // Promoting to a knight
      remove_piece(pos, m->from);
      put_piece(pos, KNIGHT, c, m->to);
      break;

    case PR_BISHOP:
      // Promoting to a bishop
      remove_piece(pos, m->from);
      put_piece(pos, BISHOP, c, m->to);
      break;

    case PR_ROOK:
      // Promoting to a rook
      remove_piece(pos, m->from);
      put_piece(pos, ROOK, c, m->to);
      break;

    case PR_QUEEN:
      // Promoting to a queen
      remove_piece(pos, m->from);
      put_piece(pos, QUEEN, c, m->to);
      break;

    case PC_KNIGHT:
      // Promoting to a knight + capturing piece
      remove_piece(pos, m->from);
      pos->history[pos->ply].captured = pos->board[m->to];
      remove_piece(pos, m->to);
      put_piece(pos, KNIGHT, c, m->to);
      break;

    case PC_BISHOP:
      // Promoting to a bishop + capturing piece
      remove_piece(pos, m->from);
      pos->history[pos->ply].captured = pos->board[m->to];
      remove_piece(pos, m->to);
      put_piece(pos, BISHOP, c, m->to);
      break;

    case PC_ROOK:
      // Promoting to a rook + capturing piece
      remove_piece(pos, m->from);
      pos->history[pos->ply].captured = pos->board[m->to];
      remove_piece(pos, m->to);
      put_piece(pos, ROOK, c, m->to);
      break;

    case PC_QUEEN:
      // Promoting to a queen + capturing piece
      remove_piece(pos, m->from);
      pos->history[pos->ply].captured = pos->board[m->to];
      remove_piece(pos, m->to);
      put_piece(pos, QUEEN, c, m->to);
      break;

    case CAPTURE:
      // Capturing piece
      pos->history[pos->ply].captured = pos->board[m->to];
      move_piece(pos, m->from, m->to);
      break;
  }
}

// Undoes the given move in the given position
void undo(Position *pos, Move *m){
  pos->side_to_play ^= BLACK;
  Color c = pos->side_to_play;
  switch (m->flag) {
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
        move_piece_quiet(pos, f8, h8);
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
      put_piece(pos, PAWN, c ^ BLACK, m->to + (c == WHITE ? -8 : 8));
      break;

    // Promoting to the given piece
    case PR_KNIGHT:
    case PR_BISHOP:
    case PR_ROOK:
    case PR_QUEEN:
      remove_piece(pos, m->to);
      put_piece(pos, PAWN, c, m->from);
      break;

    // Promoting to the given piece through a capture
    case PC_KNIGHT:
    case PC_BISHOP:
    case PC_ROOK:
    case PC_QUEEN:
      remove_piece(pos, m->to);
      put_piece(pos, PAWN, c, m->from);

      Piece pr_captured = pos->history[pos->ply].captured;
      put_piece(pos, PIECE_TO_TYPE[pr_captured], PIECE_TO_COLOR[pr_captured], m->to);
      break;

    case CAPTURE:
      // Capturing piece
      move_piece_quiet(pos, m->to, m->from);

      Piece captured = pos->history[pos->ply].captured;
      put_piece(pos, PIECE_TO_TYPE[captured], PIECE_TO_COLOR[captured], m->to);
      break;
  }
  --pos->ply;
}

// Generates all legal moves for the given position and increments pointer to
// last move in move list
Move* generate_legal_moves(Position *pos, Move *list) {
  const Color me = pos->side_to_play;
  const Color you = me ^ BLACK;

  // Occupancy bitboards for a single type of piece
  const U64 my_king = pos->pieces[me][KING];
  const U64 your_king = pos->pieces[you][KING];
  const U64 my_pawns = pos->pieces[me][PAWN];
  const U64 your_pawns = pos->pieces[you][PAWN];
  const U64 my_knights = pos->pieces[me][KNIGHT];
  const U64 your_knights = pos->pieces[you][KNIGHT];
  const U64 my_bishops = pos->pieces[me][BISHOP];
  const U64 your_bishops = pos->pieces[you][BISHOP];
  const U64 my_rooks = pos->pieces[me][ROOK];
  const U64 your_rooks = pos->pieces[you][ROOK];
  const U64 my_queens = pos->pieces[me][QUEEN];
  const U64 your_queens = pos->pieces[you][QUEEN];

  // Occupancy bitboards for pieces of a given side / type
  const U64 all_my_pieces = my_pawns | my_knights | my_bishops | my_rooks | my_queens | my_king;
  const U64 all_your_pieces = your_pawns | your_knights | your_bishops | your_rooks | your_queens | your_king;
  const U64 your_orthogonal_sliders = your_rooks | your_queens;
  const U64 your_diagonal_sliders = your_bishops | your_queens;

  // Occupancy bitboards for all pieces
  const U64 all_pieces = all_my_pieces | all_your_pieces;

  // King squares
  const Square my_king_square = get_lsb_idx(my_king);
  const Square your_king_square = get_lsb_idx(your_king);

  U64 orthogonal_pin = 0ULL;
  U64 diagonal_pin = 0ULL;

  U64 attacked = 0ULL;  // Squares king cannot move to
  U64 checkmask = 0ULL; // 1 for all pieces checking king + path to king, else all 1s;

  // General purpose bitboards
  U64 b1, b2, b3;

  int checking_pieces = 0; // Number of pieces checking king

  // Generate orthogonal pin masks + checkmasks
  if (ROOK_ATTACKS[my_king_square] & your_orthogonal_sliders) {
    U64 attackHV = get_rook_attacks(my_king_square, all_pieces) & your_orthogonal_sliders;
    U64 pinsHV = get_xray_rook_attacks(my_king_square, all_pieces) & your_orthogonal_sliders;
    while (attackHV) {
      checkmask |= PIN_BETWEEN[my_king_square][pop_lsb(&attackHV)];
      checking_pieces++;
    }
    while (pinsHV) {
      orthogonal_pin |= PIN_BETWEEN[my_king_square][pop_lsb(&pinsHV)];
    }
  }

  // Generate diagonal pin masks + checkmasks
  if (BISHOP_ATTACKS[my_king_square] & your_diagonal_sliders) {
    U64 attackD12 = get_bishop_attacks(my_king_square, all_pieces) & your_diagonal_sliders;
    U64 pinsD12 = get_xray_bishop_attacks(my_king_square, all_pieces) & your_diagonal_sliders;
    while (attackD12) {
      checkmask |= PIN_BETWEEN[my_king_square][pop_lsb(&attackD12)];
      checking_pieces++;
    }

    while (pinsD12) {
      diagonal_pin |= PIN_BETWEEN[my_king_square][pop_lsb(&pinsD12)];
    }
  }

  // Generate knight pin masks + checkmasks
  // We can only ever be in check from a single knight at once: This can only have a single bit set
  b1 = KNIGHT_ATTACKS[my_king_square] & your_knights;
  checkmask |= b1;

  // Add one to the checking pieces if we are being put in check by a knight
  checking_pieces += (b1 != 0);

  // Generate pawn checkmasks
  checkmask |= PAWN_ATTACKS[me][my_king_square] & your_pawns;

  // If no piece is checking the king then all squares should be open
  if (!checkmask) checkmask = 0xFFFFFFFFFFFFFFFF;

  const U64 moveable = ~all_my_pieces & checkmask;

  // Generate attacked squares
  attacked |= get_all_pawn_attacks(your_pawns, you);
  b1 = your_knights;
  while (b1) attacked |= KNIGHT_ATTACKS[pop_lsb(&b1)];

  // We want to make sure that the squares behind the king are attacked as well:
  // So we need to remove the king from the board
  // https://lichess.org/editor/8/8/3r1k2/8/8/8/3K4/8_w_-_-_0_1?color=white
  b1 = all_pieces ^ my_king;

  b2 = your_diagonal_sliders;
  while (b2) attacked |= get_bishop_attacks(pop_lsb(&b2), b1);
  b2 = your_orthogonal_sliders;
  while (b2) attacked |= get_rook_attacks(pop_lsb(&b2), b1);
  attacked |= KING_ATTACKS[your_king_square];

  // King moves
  list = get_moves(my_king_square, KING_ATTACKS[my_king_square] & ~attacked & ~all_my_pieces & ~all_your_pieces, list, QUIET);
  list = get_moves(my_king_square, KING_ATTACKS[my_king_square] & ~attacked & ~all_my_pieces & all_your_pieces,  list, CAPTURE);

  // If our king is in check from more than one piece, both checks cannot be blocked:
  // the king is the only piece that can move, so we can stop here.
  if (checking_pieces > 1) return list;

  // Castling
  // Generate kingside castling moves
  // if !((king / kingside rook moved) | (any pieces / attackers in the way) | (king in check))
  if (!((pos->history[pos->ply].entry & OO_MASK[me]) | ((all_pieces | attacked) & OO_BLOCKERS_MASK[me]) | (my_king & attacked))) {
    Move m;
    m.flag = OO;
    if (me == WHITE) {
      m.from = e1;
      m.to = g1;
    } else {
      m.from = e8;
      m.to = g8;
    }
    *list++ = m;
  }

  // Generate queenside castling moves
  // if !((king / queenside rook moved) | (any pieces / attackers in the way EXCEPT for the a+b files) | (king in check))
  if (!((pos->history[pos->ply].entry & OOO_MASK[me]) | ((all_pieces | (attacked & OOO_IGNORE_DANGER[me])) & OOO_BLOCKERS_MASK[me]) | (my_king & attacked))) {
    Move m;
    m.flag = OOO;
    if (me == WHITE) {
      m.from = e1;
      m.to = c1;
    } else {
      m.from = e8;
      m.to = c8;
    }
    *list++ = m;
  }

  // Pawn moves
  const U64 pawns_take = my_pawns & ~orthogonal_pin; // These pawns can take
  const U64 pawns_push = my_pawns & ~diagonal_pin;   // These pawns can push

  // Pawn moves
  b1 = all_your_pieces & NOT_H_FILE & checkmask;
  U64 pawns_take_left = pawns_take & (me == WHITE ? (b1 >> 7) : (b1 << 9));   // These pawns can take left
  b1 = all_your_pieces & NOT_A_FILE & checkmask;
  U64 pawns_take_right = pawns_take & (me == WHITE ? (b1 >> 9) : (b1 << 7));   // These pawns can take right

  U64 pawns_push_single = pawns_push & (me == WHITE ? (~all_pieces >> 8) : (~all_pieces << 8)); // These pawns can move forward

  b1 = ~all_pieces & checkmask;
  // Check pawns_push_double first: we only want to prune out checks after getting double pushes
  // Example: https://lichess.org/editor/4k3/8/8/b7/8/8/2P5/4K3_w_-_-_0_1?color=white
  // These pawns can move forward 2 squares
  U64 pawns_push_double = pawns_push_single & DOUBLE_PUSH_RANK[me] & (me == WHITE ? (b1 >> 16) : (b1 << 16));

  // Ensure pawns_push_single can move forward with the checkmask
  pawns_push_single &= (me == WHITE ? (checkmask >> 8) : (checkmask << 8));

  // Prune pin info
  // Pawns capturing a piece / moving can only do so if either:
  // 1. They are not pinned
  // 2. The piece they are capturing / square they are moving to is in the pinmask.

  // pawns_take_left and pawns_take_right have already had orthogonal pins filtered out
  // so only remove diagonal pins
  b1 = pawns_take_left & ~diagonal_pin;
  b2 = pawns_take_left & (me == WHITE ? ((diagonal_pin & NOT_H_FILE) >> 7) : ((diagonal_pin & NOT_H_FILE) << 9));
  pawns_take_left = (b1 | b2);

  b1 = pawns_take_right & ~diagonal_pin;
  b2 = pawns_take_right & (me == WHITE ? ((diagonal_pin & NOT_A_FILE) >> 9) : ((diagonal_pin & NOT_A_FILE) << 7));
  pawns_take_right = (b1 | b2);

  // pawns_push_single and pawns_push_double have already had diagonal pins filtered out
  // so only remove diagonal pins
  b1 = pawns_push_single & ~orthogonal_pin;
  b2 = pawns_push_single & (me == WHITE ? (orthogonal_pin >> 8) : (orthogonal_pin << 8));
  pawns_push_single = (b1 | b2);

  b1 = pawns_push_double & ~orthogonal_pin;
  b2 = pawns_push_double & (me == WHITE ? (orthogonal_pin >> 16) : (orthogonal_pin << 16));
  pawns_push_double = (b1 | b2);

  // Get en passant targets
  const U64 enpassant_target = SQUARE_TO_BITBOARD[pos->history[pos->ply].enpassant];

  // Add en passant moves if they exist
  if (enpassant_target) {
    // The pawn that jumped forward to allow en passant
    // Diagonally pinned pawns cannot be taken by en passant
    // https://lichess.org/editor/4k3/1b6/8/3pP3/8/5K2/8/8_w_-_-_0_1?color=white
    // https://lichess.org/editor/4k3/5b2/8/3pP3/8/1K6/8/8_w_-_-_0_1?color=white
    b1 = (me == WHITE ? (enpassant_target >> 8) : (enpassant_target << 8)) & checkmask & ~diagonal_pin;

    // Checking to see if we have a piece that can take the en passant pawn
    U64 enpassant_left  = pawns_take & NOT_A_FILE & (b1 << 1);
    U64 enpassant_right = pawns_take & NOT_H_FILE & (b1 >> 1);

    // Prune diagonal pins
    // A diagonally pinned pawn can only capture en passant if the en passant square is also along the pin
    // https://lichess.org/editor/4k3/2b5/8/3pP3/8/6K1/8/8_w_-_-_0_1?color=white
    // https://lichess.org/editor/4k3/6b1/8/3pP3/8/2K5/8/8_w_-_-_0_1?color=white

    b2 = -((enpassant_target & diagonal_pin) != 0); // all bits set if EP pawn pinned, else 0
    enpassant_left  &= (b2 | ~diagonal_pin);
    enpassant_right &= (b2 | ~diagonal_pin);

    // Prune orthogonal pins
    // We should not prune orthogonal pins if the king is aligned vertically to the piece,
    // as the pawn taking en passant will block the check.
    if (my_king & EP_RANK[me]){
        // If we remove our pawn and the EP pawn, check if one of our opponent's orthogonal sliders can see king
        b3 = get_rook_attacks(my_king_square, all_pieces ^ (b1 | enpassant_left));
        enpassant_left &= -(((b3 & your_orthogonal_sliders) | b2) == 0);

        b3 = get_rook_attacks(my_king_square, all_pieces ^ (b1 | enpassant_right));
        enpassant_right &= -(((b3 & your_orthogonal_sliders) | b2) == 0);
    }

    // All pins have been pruned, add the moves
    Move move = {.to = get_lsb_idx(enpassant_target), .flag = EN_PASSANT};
    if (enpassant_left) {
      move.from = get_lsb_idx(enpassant_left);
      *list++ = move;
    }

    if (enpassant_right) {
      move.from = get_lsb_idx(enpassant_right);
      *list++ = move;
    }
  }

  // Add all other pawn moves to move list
  Move move;

  // Pawns pushing two squares forward
  while (pawns_push_double) {
    Square s = pop_lsb(&pawns_push_double);
    move.from = s;
    move.to = (me == WHITE ? (s + 16) : (s - 16));
    move.flag = DOUBLE_PUSH;
    *list++ = move;
  }

  // Any pawns on this rank can promote this move
  const U64 promotion_rank = DOUBLE_PUSH_RANK[you];

  // Pawns pushing one square forward
  b1 = pawns_push_single & promotion_rank;
  b2 = pawns_push_single & ~promotion_rank;

  while (b1) {
    Square s = pop_lsb(&b1);
    move.from = s;
    move.to = (me == WHITE ? (s + 8) : (s - 8));
    move.flag = PR_KNIGHT;
    *list++ = move;
    move.flag = PR_BISHOP;
    *list++ = move;
    move.flag = PR_ROOK;
    *list++ = move;
    move.flag = PR_QUEEN;
    *list++ = move;
  }

  while (b2) {
    Square s = pop_lsb(&b2);
    move.from = s;
    move.to = (me == WHITE ? (s + 8) : (s - 8));
    move.flag = QUIET;
    *list++ = move;
  }

  // Pawns that can take left
  b1 = pawns_take_left & promotion_rank;
  b2 = pawns_take_left & ~promotion_rank;

  while (b1) {
    Square s = pop_lsb(&b1);
    move.from = s;
    move.to = (me == WHITE ? (s + 7) : (s - 9));
    move.flag = PC_KNIGHT;
    *list++ = move;
    move.flag = PC_BISHOP;
    *list++ = move;
    move.flag = PC_ROOK;
    *list++ = move;
    move.flag = PC_QUEEN;
    *list++ = move;
  }

  while (b2) {
    Square s = pop_lsb(&b2);
    move.from = s;
    move.to = (me == WHITE ? (s + 7) : (s - 9));
    move.flag = CAPTURE;
    *list++ = move;
  }

  // Pawns that can take right
  b1 = pawns_take_right & promotion_rank;
  b2 = pawns_take_right & ~promotion_rank;

  while (b1) {
    Square s = pop_lsb(&b1);
    move.from = s;
    move.to = (me == WHITE ? (s + 9) : (s - 7));
    move.flag = PC_KNIGHT;
    *list++ = move;
    move.flag = PC_BISHOP;
    *list++ = move;
    move.flag = PC_ROOK;
    *list++ = move;
    move.flag = PC_QUEEN;
    *list++ = move;
  }

  while (b2) {
    Square s = pop_lsb(&b2);
    move.from = s;
    move.to = (me == WHITE ? (s + 9) : (s - 7));
    move.flag = CAPTURE;
    *list++ = move;
  }

  // Generate knight moves
  // Pinned knights can never move: filter them out immediately
  // https://lichess.org/editor/8/4k3/1b6/8/3N4/4K3/8/8_w_-_-_0_1?color=white
  // https://lichess.org/editor/8/4k3/8/8/8/r1N1K3/8/8_w_-_-_0_1?color=white
  b1 = my_knights & ~(orthogonal_pin | diagonal_pin);
  while (b1) {
    Square s = pop_lsb(&b1);
    // Cannot capture our own pieces or make a move that does not stop a check (if check exists)
    b2 = KNIGHT_ATTACKS[s] & moveable;
    list = get_moves(s, b2 & ~all_your_pieces, list, QUIET);
    list = get_moves(s, b2 & all_your_pieces, list, CAPTURE);
  }

  // Best to handle pinned queens with the bishops / rooks; the lookups are the same
  // Generate bishop + diagonally pinned queen moves
  // Orthogonally pinned bishops can never move: filter them out immediately
  // https://lichess.org/editor/8/8/3k4/8/1KB2r2/8/8/8_w_-_-_0_1?color=white
  b1 = my_bishops & ~orthogonal_pin;
  b2 = (my_queens | b1) & diagonal_pin; // Diagonally pinned bishops / queens
  while (b2) {
    Square s = pop_lsb(&b2);
    b3 = get_bishop_attacks(s, all_pieces) & moveable & diagonal_pin;
    list = get_moves(s, b3 & ~all_your_pieces, list, QUIET);
    list = get_moves(s, b3 & all_your_pieces, list, CAPTURE);
  }

  b2 = b1 & ~diagonal_pin; // Non pinned bishops
  while (b2) {
    Square s = pop_lsb(&b2);
    b3 = get_bishop_attacks(s, all_pieces) & moveable;
    list = get_moves(s, b3 & ~all_your_pieces, list, QUIET);
    list = get_moves(s, b3 & all_your_pieces, list, CAPTURE);
  }

  // Generate rook + orthogonally pinned queen moves
  // Diagonally pinned rooks can never move: filter them out immediately
  // https://lichess.org/editor/8/8/3k4/4r3/8/2K3B1/8/8_w_-_-_0_1?color=white
  b1 = my_rooks & ~diagonal_pin;
  b2 = (my_queens | b1) & orthogonal_pin; // Orthogonally pinned rooks / queens
  while (b2) {
    Square s = pop_lsb(&b2);
    b3 = get_rook_attacks(s, all_pieces) & moveable & orthogonal_pin;
    list = get_moves(s, b3 & ~all_your_pieces, list, QUIET);
    list = get_moves(s, b3 & all_your_pieces, list, CAPTURE);
  }

  b2 = b1 & ~orthogonal_pin; // Non pinned rooks
  while (b2) {
    Square s = pop_lsb(&b2);
    b3 = get_rook_attacks(s, all_pieces) & moveable;
    list = get_moves(s, b3 & ~all_your_pieces, list, QUIET);
    list = get_moves(s, b3 & all_your_pieces, list, CAPTURE);
  }

  // Generate non-pinned queen moves
  // Pinned queens have already been handled with the above lookups;
  // we only need to handle the non-pinned ones
  b1 = my_queens & ~(orthogonal_pin | diagonal_pin); // All non pinned queens
  while (b1) {
    Square s = pop_lsb(&b1);
    b2 = get_queen_attacks(s, all_pieces) & moveable;
    list = get_moves(s, b2 & ~all_your_pieces, list, QUIET);
    list = get_moves(s, b2 & all_your_pieces, list, CAPTURE);
  }

  return list;
}

Move parse_move(Position *pos, const char *move_str) {
  Move move; // The move we want to return
  Move move_list[256];
  U64 n_moves;
  // All the moves in the current position
  n_moves = generate_legal_moves(pos, move_list) - move_list;

  // Get the start and end squares from the move string
  move.from = string_to_square(move_str[0], move_str[1]);
  move.to = string_to_square(move_str[2], move_str[3]);

  // Go through the move list and check if any of them are the same
  // as the move we are parsing
  for (int i = 0; i < n_moves; i++) {
    if (
      move.from == move_list[i].from &&
      move.to == move_list[i].to
    ) {
      // Return the move from the move list, as this contains
      // all the correct move flags
      return move_list[i];
    }
  }

  // The move must not be in the move list: return invalid move
  move.from = NO_SQUARE;

  return move;
}

void get_move_str(Move move, char *move_str) {
  // Initialize move string
  snprintf(move_str, MAX_MOVE_STR_LEN, "%s%s", SQUARE_TO_STRING[move.from], SQUARE_TO_STRING[move.to]);

  // Check for promotions
  if      (move.flag == PR_KNIGHT || move.flag == PC_KNIGHT) move_str[4] = 'n';
  else if (move.flag == PR_BISHOP || move.flag == PC_BISHOP) move_str[4] = 'b';
  else if (move.flag == PR_ROOK   || move.flag == PC_ROOK  ) move_str[4] = 'r';
  else if (move.flag == PR_QUEEN  || move.flag == PC_QUEEN ) move_str[4] = 'q';
  else move_str[4] = '\0';

  // Add the string termination character
  move_str[5] = '\0';

}
