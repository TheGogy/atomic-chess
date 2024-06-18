#include "movegen.h"
#include "bitboards.h"
#include "position.h"
#include "tables.h"

// Plays a move in the given position
void play(Position *pos, Move *m){
  Color c = pos->side_to_play;

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
  Color c = pos->side_to_play;
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
      put_piece(pos, PAWN, c, (c == WHITE ? -8 : 8));
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
  pos->side_to_play ^= BLACK;
  --pos->ply;
}

// Generates all legal moves for the given position and increments pointer to
// last move in move list
Move* generate_legal_moves(Position *pos, Move *list) {
  const Color me = pos->side_to_play;
  const Color you = me ^ BLACK;

  const U64 my_king = pos->pieces[me][KING];
  const U64 your_king = pos->pieces[you][KING];

  const Square my_king_square = get_lsb_idx(my_king);
  const Square your_king_square = get_lsb_idx(your_king);

  // All piece bitboards
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

  const U64 all_my_pieces = my_pawns | my_knights | my_bishops | my_rooks | my_queens | my_king;
  const U64 all_your_pieces = your_pawns | your_knights | your_bishops | your_rooks | your_queens | your_king;
  const U64 your_orthogonal_sliders = your_rooks | your_queens;
  const U64 your_diagonal_sliders = your_bishops | your_queens;

  const U64 all_pieces = all_my_pieces | all_your_pieces;

  const U64 my_double_push_rank = DOUBLE_PUSH_RANK[me];
  const U64 my_promotion_rank = DOUBLE_PUSH_RANK[you];
  const U64 my_enpassant_rank = EP_RANK[me];

  const U64 my_oo_mask = OO_MASK[me];
  const U64 my_oo_blocker_mask = OO_BLOCKERS_MASK[me];

  const U64 my_ooo_mask = OOO_MASK[me];
  const U64 my_ooo_blocker_mask = OOO_BLOCKERS_MASK[me];
  const U64 my_ooo_ignore_danger = OOO_IGNORE_DANGER[me];

  U64 orthogonal_pin = 0ULL;
  U64 diagonal_pin = 0ULL;

  U64 attacked = 0ULL; // Squares kin cannot move to
  U64 checkmask = 0ULL; // 1 for all pieces checking king, else all 1s;

  U64 enpassant_target = SQUARE_TO_BITBOARD[pos->history[pos->ply].enpassant];

  // General purpose bitboards used for move generation
  U64 b1, b2, b3;

  // Generate orthogonal pin masks
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

  // Generate diagonal pin masks
  if (PSEUDO_LEGAL_ATTACKS[BISHOP][my_king_square] & your_diagonal_sliders) {
    U64 attackD12 = get_bishop_attacks(my_king_square, all_pieces) & your_diagonal_sliders;
    U64 pinsD12 = get_xray_bishop_lookups(my_king_square, all_pieces) & your_diagonal_sliders;
    while (attackD12) {
      checkmask |= CHECK_BETWEEN[my_king_square][pop_lsb(&attackD12)];
    }

    while (pinsD12) {
      diagonal_pin |= PIN_BETWEEN[my_king_square][pop_lsb(&pinsD12)];
    }
  }

  // Generate knight pin masks
  // We can only ever be in check from a single knight at once: This can only have a single bit set
  checkmask |= PSEUDO_LEGAL_ATTACKS[KNIGHT][my_king_square] & your_knights;

  if (!checkmask) checkmask = 0xFFFFFFFFFFFFFFFF;

  const U64 moveable = ~all_my_pieces & checkmask;

  // Generate En Passant targets
  if (enpassant_target) {

    if ((my_enpassant_rank & my_king) && (my_enpassant_rank & your_orthogonal_sliders) && (my_enpassant_rank & my_pawns)) {
      const U64 enpassant_left = my_pawns & ((enpassant_target & NOT_H_FILE) >> 1);
      const U64 enpassant_right = my_pawns & ((enpassant_target & NOT_A_FILE) << 1);

      if (enpassant_left) {
        const U64 occ_without_ep = all_pieces & ~(enpassant_target | enpassant_left);
        if ((get_rook_attacks(my_king_square, occ_without_ep) & my_enpassant_rank) & your_orthogonal_sliders) {
          enpassant_target = 0ULL;
        }
      }
      if (enpassant_right) {
        const U64 occ_without_ep = all_pieces & ~(enpassant_target | enpassant_right);
        if ((get_rook_attacks(my_king_square, occ_without_ep) & my_enpassant_rank) & your_orthogonal_sliders) {
          enpassant_target = 0ULL;
        }
      }
    }
  }

  // Generate attacked squares and add them to the king ban
  // We can't use checkmask because it filters out attackers that cannot directly see king: king may walk into checks.
  attacked |= get_all_knight_attacks(your_knights);
  attacked |= get_all_pawn_attacks(pos, you);
  attacked |= KING_ATTACKS[your_king_square];
  b1 = your_orthogonal_sliders;
  b2 = your_diagonal_sliders;
  while (b1) attacked |= get_rook_attacks(pop_lsb(&b1), all_pieces ^ my_king);
  while (b2) attacked |= get_bishop_attacks(pop_lsb(&b2), all_pieces ^ my_king);

  // King can move to all squares except attacked ones / ones with our pieces
  b1 = KING_ATTACKS[my_king_square] & ~(all_my_pieces | attacked);
  list = get_moves(my_king_square, b1 & ~all_your_pieces, list, QUIET);
  list = get_moves(my_king_square, b1 & all_your_pieces, list, CAPTURE);

  // Generate kingside castling moves
  if (!((pos->history[pos->ply].entry & my_oo_mask) | ((all_pieces | attacked) & my_oo_blocker_mask))) {
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
  if (!((pos->history[pos->ply].entry & my_ooo_mask) | ((all_pieces | (attacked & my_ooo_ignore_danger)) & my_ooo_blocker_mask))) {
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
  b1 = my_knights & ~(orthogonal_pin | diagonal_pin);
  while (b1) {
    Square s = pop_lsb(&b1);
    // Cannot capture our own pieces or make a move that does not stop a check (if check exists)
    b2 = KNIGHT_ATTACKS[s] & moveable;
    list = get_moves(s, b2 & ~all_your_pieces, list, QUIET);
    list = get_moves(s, b2 & all_your_pieces, list, CAPTURE);
  }

  // Best to handle pinned queens with the bishops / rooks; the lookups are the same
  // Generate bishop moves
  // Orthogonally pinned bishops can never move: filter them out immediately
  b1 = my_bishops & ~orthogonal_pin;
  b2 = (my_queens | b2) & diagonal_pin; // Diagonally pinned bishops and queens
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

  // Generate rook moves
  // Diagonally pinned rooks can never move: filter them out immediately
  b1 = my_rooks & ~diagonal_pin;
  b2 = (my_queens | b1) & orthogonal_pin; // Orthogonally pinned rooks and queens
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

  // Generate queen moves
  // Pinned queens have already been handled with the above lookups;
  // we only need to handle the non-pinned ones
  b1 &= ~(orthogonal_pin | diagonal_pin); // b1 already contains queen moves: This is all non pinned queens
  while (b1) {
    Square s = pop_lsb(&b1);
    b2 = get_queen_attacks(s, all_pieces) & moveable;
    list = get_moves(s, b2 & ~all_your_pieces, list, QUIET);
    list = get_moves(s, b2 & all_your_pieces, list, CAPTURE);
  }

  // Generate pawn moves
  // Horizontally pinned pawns can never move: filter them out immediately
  b1 = my_pawns & ~(orthogonal_pin & RANK_MASKS[my_king_square / 8]);

  // Horizontally pins have been filtered: This is only vertically pinned pawns
  // Vertically pinned pawns cannot take, only push.
  // Vertically pinned pawns also cannot promote: either the king, or the piece pinning them,
  // will always block the promotion square.
  //
  // https://lichess.org/editor/3r2k1/3P4/3K4/8/8/8/8/8_w_-_-_0_1?color=white
  // https://lichess.org/editor/3K2k1/3P4/8/3r4/8/8/8/8_w_-_-_0_1?color=white

  b2 = b1 & orthogonal_pin;
  b3 = b2 & my_double_push_rank;
  b2 ^= b3; // Do not double count the pawns

  // All other vertically pinned pawns
  while (b2) {
    Square from = pop_lsb(&b2);
    Move move = {.flags = QUIET, .from = from};
    U64 target_bb = (me == WHITE ? (SQUARE_TO_BITBOARD[from] << 8) : (SQUARE_TO_BITBOARD[from] >> 8)) & moveable;
    if (target_bb) {
      move.to = get_lsb_idx(target_bb);
      *list++ = move;
    }
  }

  // All vertically pinned pawns that can double push
  while (b3) {
    Square from = pop_lsb(&b3);
    Move move = {.flags = QUIET, .from = from};
    // Single push
    U64 target_bb = (me == WHITE ? (SQUARE_TO_BITBOARD[from] << 8) : (SQUARE_TO_BITBOARD[from] >> 8)) & moveable;
    if (target_bb) {
      move.to = get_lsb_idx(target_bb);
      *list++ = move;
    }

    target_bb = (me == WHITE ? (SQUARE_TO_BITBOARD[from] << 16) : (SQUARE_TO_BITBOARD[from] >> 16)) & moveable;
    if (target_bb) {
      move.to = get_lsb_idx(target_bb);
      *list++ = move;
    }
  }

  // All other pawns
  b2 = b1 & ~orthogonal_pin;
  b3 = b2 & my_double_push_rank;
  b2 ^= b3; // Do not double count the pawns

  // All other pawns that can double push (As they can double push, they must not be able to promote)
  while (b3) {
    Square from = pop_lsb(&b3);

    Move move = {.flags = QUIET, .from = from};

    // Single push
    U64 target_bb = (me == WHITE ? (SQUARE_TO_BITBOARD[from] << 8) : (SQUARE_TO_BITBOARD[from] >> 8)) & moveable;
    if (target_bb) {
      move.to = get_lsb_idx(target_bb);
      *list++ = move;
    }

    // Double push
    target_bb = (me == WHITE ? (SQUARE_TO_BITBOARD[from] << 16) : (SQUARE_TO_BITBOARD[from] >> 16)) & moveable;
    if (target_bb) {
      move.to = get_lsb_idx(target_bb);
      move.flags = DOUBLE_PUSH;
      *list++ = move;
    }

    // Capture right
    target_bb = (me == WHITE ? (SQUARE_TO_BITBOARD[from] << 9) : (SQUARE_TO_BITBOARD[from] >> 7)) & moveable & all_your_pieces;
    if (target_bb) {
      move.to = get_lsb_idx(target_bb);
      move.flags = CAPTURE;
      *list++ = move;
    }

    // Capture left
    target_bb = (me == WHITE ? (SQUARE_TO_BITBOARD[from] << 7) : (SQUARE_TO_BITBOARD[from] >> 9)) & moveable & all_your_pieces;
    if (target_bb) {
      move.to = get_lsb_idx(target_bb);
      move.flags = CAPTURE;
      *list++ = move;
    }
  }

  // All my pawns that can promote
  b3 = b2 & my_promotion_rank;
  b2 ^= b3; // Do not double count the pawns
  while (b3) {
    Square from = pop_lsb(&b3);

    Move move = {.flags = QUIET, .from = from};

    // Push
    U64 target_bb = (me == WHITE ? (SQUARE_TO_BITBOARD[from] << 8) : (SQUARE_TO_BITBOARD[from] >> 8)) & moveable;
    if (target_bb) {
      move.to = get_lsb_idx(target_bb);
      move.flags = PR_KNIGHT;
      *list++ = move;
      move.flags = PR_BISHOP;
      *list++ = move;
      move.flags = PR_ROOK;
      *list++ = move;
      move.flags = PR_QUEEN;
      *list++ = move;
    }

    // Capture right
    target_bb = (me == WHITE ? (SQUARE_TO_BITBOARD[from] << 9) : (SQUARE_TO_BITBOARD[from] >> 7)) & moveable & all_your_pieces;
    if (target_bb) {
      move.to = get_lsb_idx(target_bb);
      move.flags = PC_KNIGHT;
      *list++ = move;
      move.flags = PC_BISHOP;
      *list++ = move;
      move.flags = PC_ROOK;
      *list++ = move;
      move.flags = PC_QUEEN;
      *list++ = move;
    }

    // Capture left
    target_bb = (me == WHITE ? (SQUARE_TO_BITBOARD[from] << 7) : (SQUARE_TO_BITBOARD[from] >> 9)) & moveable & all_your_pieces;
    if (target_bb) {
      move.to = get_lsb_idx(target_bb);
      move.flags = PC_KNIGHT;
      *list++ = move;
      move.flags = PC_BISHOP;
      *list++ = move;
      move.flags = PC_ROOK;
      *list++ = move;
      move.flags = PC_QUEEN;
      *list++ = move;
    }
  }

  // All the remaining pawns
  while (b2) {
    Square from = pop_lsb(&b2);

    Move move = {.flags = QUIET, .from = from};

    U64 target_bb = (me == WHITE ? (SQUARE_TO_BITBOARD[from] << 8) : (SQUARE_TO_BITBOARD[from] >> 8)) & moveable;
    if (target_bb) {
      move.to = get_lsb_idx(target_bb);
      *list++ = move;
    }

    // Capture right
    target_bb = (me == WHITE ? (SQUARE_TO_BITBOARD[from] << 9) : (SQUARE_TO_BITBOARD[from] >> 7)) & moveable & all_your_pieces;
    if (target_bb) {
      move.to = get_lsb_idx(target_bb);
      move.flags = CAPTURE;
      *list++ = move;
    }

    // Capture left
    target_bb = (me == WHITE ? (SQUARE_TO_BITBOARD[from] << 7) : (SQUARE_TO_BITBOARD[from] >> 9)) & moveable & all_your_pieces;
    if (target_bb) {
      move.to = get_lsb_idx(target_bb);
      move.flags = CAPTURE;
      *list++ = move;
    }
  }

  // Add en passant moves if they exist
  // We have already pruned out all pins; enpassant_target would be empty if en passant piece has been pinned
  if (enpassant_target) {
    Square to = get_lsb_idx(enpassant_target);
    Move move = {.flags = EN_PASSANT, .to = to};
    // Capture right
    U64 target_bb = (me == WHITE ? (SQUARE_TO_BITBOARD[to] >> 9) : (SQUARE_TO_BITBOARD[to] << 7)) & my_pawns;
    if (target_bb) {
      move.from = get_lsb_idx(target_bb);
      *list++ = move;
    }
    // Capture left
    target_bb = (me == WHITE ? (SQUARE_TO_BITBOARD[to] >> 7) : (SQUARE_TO_BITBOARD[to] << 9)) & my_pawns;
    if (target_bb) {
      move.from = get_lsb_idx(target_bb);
      *list++ = move;
    }
  }

  return list;
}
