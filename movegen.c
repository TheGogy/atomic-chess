#include "bitboards.h"
#include "movegen.h"
#include "position.h"
#include "sliders.h"
#include "tables.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// Capturing pieces for atomic chess.
// Removes all pieces in a 3x3 square, centered on "center".

// We need to store these pieces in the format:
// (4 bits allocated to each piece: there are 13 possible piece types,
// 6 pieces for each side + no piece)
// [capturing piece][captured piece][p1][p2][p3][p4]
inline void atomic_capture(Position *pos, Square center, Square attacker_square) {
    // Add the attacker and defender to the pieces taken
    U64 pieces_taken = (U64)pos->board[attacker_square] | ((U64)pos->board[center] << 4);

    // Remove the pieces from the board
    remove_piece(pos, attacker_square);
    remove_piece(pos, center);

    // All other pieces that will be taken
    U64 explosion = KING_ATTACKS[center];

    // We have already shifted over two places: attacker and attacked piece
    int i = 2;
    while (explosion) {
        // Get the square and piece
        Square s = pop_lsb(&explosion);
        Piece p = pos->board[s];

        // Pawns are not taken, unless they are the capturing piece / piece being captured
        if (!(PIECE_TO_TYPE[p] == PAWN || PIECE_TO_TYPE[p] == NO_TYPE)) {
            remove_piece(pos, s);
            // Shift the piece over by the required amount and put it into place
            pieces_taken |= ((U64)p << (i * 4));
        }
        i++;
    }

    pos->history[pos->ply].captured = pieces_taken;
}

// Unpacks the capture info and adds the pieces in to their correct squares.
inline void atomic_undo_capture(Position *pos, Square center, Square attacker_square) {

    // printf("UNDOING CAPTURE. center: %s -- attacker_square: %s\n", SQUARE_TO_STRING[center], SQUARE_TO_STRING[attacker_square]);

    U64 pieces_taken = pos->history[pos->ply].captured;
    U64 explosion = KING_ATTACKS[center];

    // Add in attacker and captured piece
    const Piece attacker = (Piece)(pieces_taken & 0xF);
    put_piece(pos, PIECE_TO_TYPE[attacker], PIECE_TO_COLOR[attacker], attacker_square);
    pieces_taken >>= 4;
    // printf("Placing attacker: %c on square %s\n", PIECE_TO_CHAR[attacker], SQUARE_TO_STRING[attacker_square]);


    const Piece captured = (Piece)(pieces_taken & 0xF);
    // We have to double check that there is a piece in the captured slot.
    // This is because when we take en passant, the captured piece is not actually
    // on that square, and so is not stored here.
    if (likely(captured)) {
        put_piece(pos, PIECE_TO_TYPE[captured], PIECE_TO_COLOR[captured], center);
        // printf("Placing captured: %c on square %s\n", PIECE_TO_CHAR[captured], SQUARE_TO_STRING[center]);
    }
    pieces_taken >>= 4;

    while (explosion) {
        Square s = pop_lsb(&explosion);
        Piece p = (Piece)(pieces_taken & 0xF);
        pieces_taken >>= 4;
        // If p is empty, do nothing
        if (p == NO_PIECE) continue;
        put_piece(pos, PIECE_TO_TYPE[p], PIECE_TO_COLOR[p], s);
    }
}

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
    pos->history[pos->ply].enpassant = NO_SQUARE;

#ifdef ATOMIC
    // For atomic history, we keep a record of pieces in the following format:
    // (4 bits allocated to each piece)
    // [capturing piece][captured piece][p1][p2][p3][p4] ...
    pos->history[pos->ply].captured = 0ULL;
#else
    // For standard chess, only a single piece can be captured at once: we only need to remove that one.
    pos->history[pos->ply].captured = NO_PIECE;
#endif // ATOMIC

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
#ifdef ATOMIC
        // For atomic en passants, both pawns are destroyed.
        remove_piece(pos, m->to + (c == WHITE ? -8 : 8));
        atomic_capture(pos, m->to, m->from);
#else
        move_piece_quiet(pos, m->from, m->to);
        remove_piece(pos, m->to + (c == WHITE ? -8 : 8));
#endif
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
#ifdef ATOMIC
        // Promotion captures in atomic lead to both pieces being destroyed
        atomic_capture(pos, m->to, m->from);
#else
        remove_piece(pos, m->from);
        pos->history[pos->ply].captured = pos->board[m->to];
        remove_piece(pos, m->to);
#endif // ATOMIC
        put_piece(pos, KNIGHT, c, m->to);
        break;

        case PC_BISHOP:
        // Promoting to a bishop + capturing piece
#ifdef ATOMIC
        // Promotion captures in atomic lead to both pieces being destroyed
        atomic_capture(pos, m->to, m->from);
#else
        remove_piece(pos, m->from);
        pos->history[pos->ply].captured = pos->board[m->to];
        remove_piece(pos, m->to);
#endif // ATOMIC
        put_piece(pos, BISHOP, c, m->to);
        break;

        case PC_ROOK:
        // Promoting to a rook + capturing piece
#ifdef ATOMIC
        // Promotion captures in atomic lead to both pieces being destroyed
        atomic_capture(pos, m->to, m->from);
#else
        remove_piece(pos, m->from);
        pos->history[pos->ply].captured = pos->board[m->to];
        remove_piece(pos, m->to);
#endif // ATOMIC
        put_piece(pos, ROOK, c, m->to);
        break;

        case PC_QUEEN:
        // Promoting to a queen + capturing piece
#ifdef ATOMIC
        // Promotion captures in atomic lead to both pieces being destroyed
        atomic_capture(pos, m->to, m->from);
#else
        remove_piece(pos, m->from);
        pos->history[pos->ply].captured = pos->board[m->to];
        remove_piece(pos, m->to);
#endif // ATOMIC
        put_piece(pos, QUEEN, c, m->to);
        break;

        case CAPTURE:
        // Capturing piece
#ifdef ATOMIC
        atomic_capture(pos, m->to, m->from);
#else
        pos->history[pos->ply].captured = pos->board[m->to];
        move_piece(pos, m->from, m->to);
#endif

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

#ifdef ATOMIC
        atomic_undo_capture(pos, m->to, m->from);
        put_piece(pos, PAWN, c ^ BLACK, m->to + (c == WHITE ? -8 : 8));
#else
        move_piece_quiet(pos, m->to, m->from);
        put_piece(pos, PAWN, c ^ BLACK, m->to + (c == WHITE ? -8 : 8));
#endif // ATOMIC
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
#ifdef ATOMIC
        atomic_undo_capture(pos, m->to, m->from);
#else
        remove_piece(pos, m->to);
        put_piece(pos, PAWN, c, m->from);

        Piece pr_captured = pos->history[pos->ply].captured;
        put_piece(pos, PIECE_TO_TYPE[pr_captured], PIECE_TO_COLOR[pr_captured], m->to);
#endif // ATOMIC
        break;

        case CAPTURE:
#ifdef ATOMIC
        atomic_undo_capture(pos, m->to, m->from);
#else
        // Capturing piece
        move_piece_quiet(pos, m->to, m->from);

        Piece captured = pos->history[pos->ply].captured;
        put_piece(pos, PIECE_TO_TYPE[captured], PIECE_TO_COLOR[captured], m->to);
#endif // ATOMIC
        break;
    }
    --pos->ply;
}

// Generates all legal moves for the given position and increments pointer to
// last move in move list
Move* generate_legal_moves_standard(Position *pos, Move *list) {
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

    // King square
    const Square my_king_square = get_lsb_idx(my_king);

    U64 orthogonal_pin = 0ULL;
    U64 diagonal_pin = 0ULL;

    U64 attacked = 0ULL;  // Squares king cannot move to
    U64 checkmask = 0ULL; // 1 for all pieces checking king + path to king, else all 1s;

    // General purpose bitboards
    U64 b1, b2, b3;

    int checking_pieces = 0; // Number of pieces checking king

    // Generate orthogonal pin masks + checkmasks
    if (likely(ROOK_ATTACKS[my_king_square] & your_orthogonal_sliders)) {
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
    if (likely(BISHOP_ATTACKS[my_king_square] & your_diagonal_sliders)) {
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
    if (likely(!checkmask)) checkmask = 0xFFFFFFFFFFFFFFFF;

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

    attacked |= KING_ATTACKS[get_lsb_idx(your_king)];

    // King moves
    list = get_moves(my_king_square, KING_ATTACKS[my_king_square] & ~attacked & ~all_my_pieces & ~all_your_pieces, list, QUIET);
    list = get_moves(my_king_square, KING_ATTACKS[my_king_square] & ~attacked & ~all_my_pieces & all_your_pieces,  list, CAPTURE);

    // If our king is in check from more than one piece, both checks cannot be blocked:
    // the king is the only piece that can move, so we can stop here.
    if (checking_pieces > 1) return list;

    // Castling
    // Generate kingside castling moves
    // if !((king / kingside rook moved) | (any pieces / attackers in the way) | (king in check))
    if (unlikely(!((pos->history[pos->ply].entry & OO_MASK[me]) | ((all_pieces | attacked) & OO_BLOCKERS_MASK[me]) | (my_king & attacked)))) {
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
    if (unlikely(!((pos->history[pos->ply].entry & OOO_MASK[me]) | ((all_pieces | (attacked & OOO_IGNORE_DANGER[me])) & OOO_BLOCKERS_MASK[me]) | (my_king & attacked)))) {
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
    if (unlikely(enpassant_target)) {
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

// Checks if a move is legal with atomic rules, and if it is then it adds it to
// the move list. Otherwise, it does nothing.
inline Move* atomic_add_move(Move *list, Color me, Color you, U64 all_pieces,
                             U64 all_pawns, U64 all_your_pieces,
                             U64 your_orthogonal_sliders,
                             U64 your_diagonal_sliders, U64 your_knights,
                             U64 your_pawns, Square from_sq, Square to_sq,
                             Square my_king_square,
                             int include_promotions, int include_enpassant) {

    const U64 from = SQUARE_TO_BITBOARD[from_sq];
    const U64 to = SQUARE_TO_BITBOARD[to_sq];
    const U64 explosion = to | KING_ATTACKS[to_sq];

    // All pieces after capture.
    // We want to remove the attacker, the defender, and all
    // other pieces.
    const U64 relevant_pieces =
        all_pieces ^ (from | to | ((all_pieces ^ all_pawns) & explosion));

    // Do not add the move if it allows the king to be in check: return immediately
    if (unlikely(get_rook_attacks(my_king_square, relevant_pieces) &
                 (your_orthogonal_sliders & relevant_pieces))) {
        return list;
    }

    if (unlikely(get_bishop_attacks(my_king_square, relevant_pieces) &
                 (your_diagonal_sliders & relevant_pieces))) {
        return list;
    }

    if (unlikely(KNIGHT_ATTACKS[my_king_square] & (your_knights & relevant_pieces))) {
        return list;
    }

    if (unlikely(PAWN_ATTACKS[you][my_king_square] & (your_pawns & relevant_pieces))) {
        return list;
    }

    // Add promotion info if required, then return
    if (unlikely(include_promotions && (from & DOUBLE_PUSH_RANK[you]))) {
        Move m;
        m.from = from_sq;
        m.to = (Square)(me == WHITE ? (from_sq + 8) : (from_sq - 8));
        if (unlikely(to & all_your_pieces)) {
            m.flag = PC_KNIGHT;
            *list++ = m;
            m.flag = PC_BISHOP;
            *list++ = m;
            m.flag = PC_ROOK;
            *list++ = m;
            m.flag = PC_QUEEN;
            *list++ = m;
        } else {
            m.flag = PR_KNIGHT;
            *list++ = m;
            m.flag = PR_BISHOP;
            *list++ = m;
            m.flag = PR_ROOK;
            *list++ = m;
            m.flag = PR_QUEEN;
            *list++ = m;
        }
        return list;
    }

    // Add enpassant info if required, then return
    if (unlikely(include_enpassant)) {
        Move m = {.flag = EN_PASSANT, .from = from_sq, .to = to_sq};
        *list++ = m;
        return list;
    }

    // Add the move
    Move m = {.flag = CAPTURE, .from = from_sq, .to = to_sq};
    *list++ = m;

    return list;
}

// Generates all legal moves for the given position and increments pointer to
// last move in move list
Move* generate_legal_moves_atomic(Position *pos, Move *list) {

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

    // King square
    const Square my_king_square = get_lsb_idx(my_king);

    // General purpose bitboards for move generation
    U64 b1, b2, b3;

    // Get attacked squares
    U64 attacked = get_all_pawn_attacks(your_pawns, you);
    b1 = your_knights;
    while (b1) attacked |= KNIGHT_ATTACKS[pop_lsb(&b1)];
    b1 = all_pieces ^ my_king;
    b2 = your_diagonal_sliders;
    while (b2) attacked |= get_bishop_attacks(pop_lsb(&b2), b1);
    b2 = your_orthogonal_sliders;
    while (b2) attacked |= get_rook_attacks(pop_lsb(&b2), b1);

    // We do not need to add king attacks: in atomic chess, the kings can touch.
    // This is because kings cannot take, as they would explode.

    list = get_moves(my_king_square, KING_ATTACKS[my_king_square] & ~attacked & ~all_my_pieces & ~all_your_pieces,  list, QUIET);

    // Castling
    // Generate kingside castling moves
    // if !((king / kingside rook moved) | (any pieces / attackers in the way) | (king in check))
    if (unlikely(!((pos->history[pos->ply].entry & OO_MASK[me]) | ((all_pieces | attacked) & OO_BLOCKERS_MASK[me]) | (my_king & attacked)))) {
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
    if (unlikely(!((pos->history[pos->ply].entry & OOO_MASK[me]) | ((all_pieces | (attacked & OOO_IGNORE_DANGER[me])) & OOO_BLOCKERS_MASK[me]) | (my_king & attacked)))) {
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

    // Generate pawn moves
    // Pushes
    b1 = my_pawns & (me == WHITE ? (~all_pieces >> 8) : (~all_pieces << 8)); // These pieces can push once
    b2 = b1 & DOUBLE_PUSH_RANK[me] & (me == WHITE ? (~all_pieces >> 16) : (~all_pieces << 16)); // These pawns can double push

    // Double pushes
    while (b2) {
        Square from = pop_lsb(&b2);
        Square to = from + (me == WHITE ? 16 : -16);
        // Check to see if piece is pinned
        if (likely(
            !( // Not pinned
                get_bishop_attacks(my_king_square, all_pieces ^ SQUARE_TO_BITBOARD[from]) & your_diagonal_sliders ||
                get_rook_attacks(my_king_square, all_pieces ^ SQUARE_TO_BITBOARD[from]) & your_orthogonal_sliders
            )
            || // or
            !( // Gets in the way of a pin
                get_bishop_attacks(my_king_square, all_pieces ^ (SQUARE_TO_BITBOARD[from] | SQUARE_TO_BITBOARD[to])) & your_diagonal_sliders ||
                get_rook_attacks(my_king_square, all_pieces ^ (SQUARE_TO_BITBOARD[from] | SQUARE_TO_BITBOARD[to])) & your_orthogonal_sliders
            )
        )) {
            // Piece is not pinned, add the move
            Move m = {.flag = DOUBLE_PUSH, .from = from, .to = to};
            *list++ = m;
        }

    }

    // Single pushes
    while (b1) {
        Square from = pop_lsb(&b1);
        Square to = from + (me == WHITE ? 8 : -8);
        // Check to see if piece is pinned
        if (likely(
            !( // Not pinned
                get_bishop_attacks(my_king_square, all_pieces ^ SQUARE_TO_BITBOARD[from]) & your_diagonal_sliders ||
                get_rook_attacks(my_king_square, all_pieces ^ SQUARE_TO_BITBOARD[from]) & your_orthogonal_sliders
            )
            || // or
            !( // Gets in the way of a pin
                get_bishop_attacks(my_king_square, all_pieces ^ (SQUARE_TO_BITBOARD[from] | SQUARE_TO_BITBOARD[to])) & your_diagonal_sliders ||
                get_rook_attacks(my_king_square, all_pieces ^ (SQUARE_TO_BITBOARD[from] | SQUARE_TO_BITBOARD[to])) & your_orthogonal_sliders
            )
        )) {
            // Piece is not pinned, add the move
            Move m = {.flag = QUIET, .from = from, .to = to};
            *list++ = m;
        }
    }

    // Pawn captures left
    b2 = my_pawns & (me == WHITE ? (all_your_pieces >> 7) : (all_your_pieces << 9)) & NOT_A_FILE; // Capture left
    while (b2) {
        Square from = pop_lsb(&b2);
        Square to = from + (me == WHITE ? 7 : -9);
        list = atomic_add_move(list, me, you, all_pieces, my_pawns | your_pawns,
                               all_your_pieces, your_orthogonal_sliders,
                               your_diagonal_sliders, your_knights, your_pawns,
                               from, to, my_king_square, 1, 0);
    }

    // Pawn captures right
    b2 = my_pawns & (me == WHITE ? (all_your_pieces >> 9) : (all_your_pieces << 7)) & NOT_H_FILE; // Capture left
    while (b2) {
        Square from = pop_lsb(&b2);
        Square to = from + (me == WHITE ? 9 : -7);
        list = atomic_add_move(list, me, you, all_pieces, my_pawns | your_pawns,
                               all_your_pieces, your_orthogonal_sliders,
                               your_diagonal_sliders, your_knights, your_pawns,
                               from, to, my_king_square, 1, 0);
    }

    // Get en passant targets
    const U64 enpassant_target = SQUARE_TO_BITBOARD[pos->history[pos->ply].enpassant];

    // Add en passant moves if they exist
    if (unlikely(enpassant_target)) {
        b1 = (me == WHITE ? (enpassant_target >> 8) : (enpassant_target << 8));
        // The EP pawn's current square, and the square we can capture EP on.
        b2 = (enpassant_target | b1);

        // Capturing left
        b3 = my_pawns & NOT_H_FILE & (b1 >> 1);
        if (unlikely(b3)) {
            Square from = get_lsb_idx(b3);
            Square to = pos->history[pos->ply].enpassant;
            // Flip the bits in the required bitboards to make it seem like the pawn
            // only moved a single square forward before passing it to the function.
            list = atomic_add_move(list, me, you, all_pieces ^ b2, my_pawns | (your_pawns ^ b2),
                                   all_your_pieces ^ b2, your_orthogonal_sliders,
                                   your_diagonal_sliders, your_knights, your_pawns ^ b2,
                                   from, to, my_king_square, 0, 1);
        }

        // Capturing right 
        b3 = my_pawns & NOT_A_FILE & (b1 << 1);
        if (unlikely(b3)) {
            Square from = get_lsb_idx(b3);
            Square to = pos->history[pos->ply].enpassant;
            // Flip the bits in the required bitboards to make it seem like the pawn
            // only moved a single square forward before passing it to the function.
            list = atomic_add_move(list, me, you, all_pieces ^ b2, my_pawns | (your_pawns ^ b2),
                                   all_your_pieces ^ b2, your_orthogonal_sliders,
                                   your_diagonal_sliders, your_knights, your_pawns ^ b2,
                                   from, to, my_king_square, 0, 1);
        }
    }

    // Add knight moves
    b1 = my_knights;
    while (b1) {
        Square from = pop_lsb(&b1);
        b2 = KNIGHT_ATTACKS[from] & ~all_my_pieces;
        while (b2) {
            Square to = pop_lsb(&b2);

            if (unlikely(SQUARE_TO_BITBOARD[to] & all_your_pieces)) {
                list = atomic_add_move(list, me, you, all_pieces, my_pawns | your_pawns,
                                       all_your_pieces, your_orthogonal_sliders,
                                       your_diagonal_sliders, your_knights, your_pawns,
                                       from, to, my_king_square, 0, 0);
            } else {
                if (likely(
                    !( // Not pinned
                        get_bishop_attacks(my_king_square, all_pieces ^ SQUARE_TO_BITBOARD[from]) & your_diagonal_sliders ||
                        get_rook_attacks(my_king_square, all_pieces ^ SQUARE_TO_BITBOARD[from]) & your_orthogonal_sliders
                    )
                    || // or
                    !( // Gets in the way of a pin
                        get_bishop_attacks(my_king_square, all_pieces ^ (SQUARE_TO_BITBOARD[from] | SQUARE_TO_BITBOARD[to])) & your_diagonal_sliders ||
                        get_rook_attacks(my_king_square, all_pieces ^ (SQUARE_TO_BITBOARD[from] | SQUARE_TO_BITBOARD[to])) & your_orthogonal_sliders
                    )
                )) {
                    // Piece is not pinned, add the move
                    Move m = {.flag = QUIET, .from = from, .to = to};
                    *list++ = m;
                }
            }
        }
    }

    // Add bishop + diagonal queen moves
    b1 = my_bishops | my_queens;
    while (b1) {
        Square from = pop_lsb(&b1);
        b2 = get_bishop_attacks(from, all_pieces) & ~all_my_pieces;
        while (b2) {
            Square to = pop_lsb(&b2);

            if (unlikely(SQUARE_TO_BITBOARD[to] & all_your_pieces)) {
                list = atomic_add_move(list, me, you, all_pieces, my_pawns | your_pawns,
                                       all_your_pieces, your_orthogonal_sliders,
                                       your_diagonal_sliders, your_knights, your_pawns,
                                       from, to, my_king_square, 0, 0);
            } else {
                if (likely(
                    !( // Not pinned
                        get_bishop_attacks(my_king_square, all_pieces ^ SQUARE_TO_BITBOARD[from]) & your_diagonal_sliders ||
                        get_rook_attacks(my_king_square, all_pieces ^ SQUARE_TO_BITBOARD[from]) & your_orthogonal_sliders
                    )
                    || // or
                    !( // Gets in the way of a pin
                        get_bishop_attacks(my_king_square, all_pieces ^ (SQUARE_TO_BITBOARD[from] | SQUARE_TO_BITBOARD[to])) & your_diagonal_sliders ||
                        get_rook_attacks(my_king_square, all_pieces ^ (SQUARE_TO_BITBOARD[from] | SQUARE_TO_BITBOARD[to])) & your_orthogonal_sliders
                    )
                )) {
                    // Piece is not pinned, add the move
                    Move m = {.flag = QUIET, .from = from, .to = to};
                    *list++ = m;
                }
            }
        }
    }

    // Add rook + orthogonal queen moves
    b1 = my_rooks | my_queens;
    while (b1) {
        Square from = pop_lsb(&b1);
        b2 = get_rook_attacks(from, all_pieces) & ~all_my_pieces;
        while (b2) {
            Square to = pop_lsb(&b2);

            if (unlikely(SQUARE_TO_BITBOARD[to] & all_your_pieces)) {
                list = atomic_add_move(list, me, you, all_pieces, my_pawns | your_pawns,
                                       all_your_pieces, your_orthogonal_sliders,
                                       your_diagonal_sliders, your_knights, your_pawns,
                                       from, to, my_king_square, 0, 0);
            } else {
                if (likely(
                    !( // Not pinned
                        get_bishop_attacks(my_king_square, all_pieces ^ SQUARE_TO_BITBOARD[from]) & your_diagonal_sliders ||
                        get_rook_attacks(my_king_square, all_pieces ^ SQUARE_TO_BITBOARD[from]) & your_orthogonal_sliders
                    )
                    || // or
                    !( // Gets in the way of a pin
                        get_bishop_attacks(my_king_square, all_pieces ^ (SQUARE_TO_BITBOARD[from] | SQUARE_TO_BITBOARD[to])) & your_diagonal_sliders ||
                        get_rook_attacks(my_king_square, all_pieces ^ (SQUARE_TO_BITBOARD[from] | SQUARE_TO_BITBOARD[to])) & your_orthogonal_sliders
                    )
                )) {
                    // Piece is not pinned, add the move
                    Move m = {.flag = QUIET, .from = from, .to = to};
                    *list++ = m;
                }
            }
        }
    }

    return list;
}

Move* generate_legal_moves(Position *pos, Move *list) {
#ifdef ATOMIC
    return generate_legal_moves_atomic(pos, list);
#else
    return generate_legal_moves_standard(pos, list);
#endif // ATOMIC
}

Move parse_move(Position *pos, const char *move_str) {

    Move move; // The move we want to return
    Move move_list[256];
    U64 n_moves;

    int is_promotion = 0;

    // All the moves in the current position
    n_moves = generate_legal_moves(pos, move_list) - move_list;

    // Get the start and end squares from the move string
    move.from = string_to_square(move_str[0], move_str[1]);
    move.to = string_to_square(move_str[2], move_str[3]);

    if (strlen(move_str) == 5 && isalpha(move_str[4])) {
        is_promotion = 1;
    }

    // Go through the move list and check if any of them are the same
    // as the move we are parsing
    for (int i = 0; i < n_moves; i++) {
        if (
            move.from == move_list[i].from &&
            move.to == move_list[i].to
        ) {
            // If it isn't a promotion, just return move
            if (!is_promotion) return move_list[i];

            if ((move.from % 8) == (move.to % 8)) {
                switch (move_str[4]) {
                    case 'n': move.flag = PR_KNIGHT; break;
                    case 'b': move.flag = PR_BISHOP; break;
                    case 'r': move.flag = PR_ROOK;   break;
                    case 'q': move.flag = PR_QUEEN;  break;
                }
            } else {
                switch (move_str[4]) {
                    case 'n': move.flag = PC_KNIGHT; break;
                    case 'b': move.flag = PC_BISHOP; break;
                    case 'r': move.flag = PC_ROOK;   break;
                    case 'q': move.flag = PC_QUEEN;  break;
                }
            }
            return move;
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
