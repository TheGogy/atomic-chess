#pragma once

#ifndef MOVEGEN_H
#define MOVEGEN_H
#include "tables.h"
#include "bitboards.h"
#include "position.h"

#define MAX_MOVE_STR_LEN 6

typedef enum moveflag {
    QUIET = 0x0,
    DOUBLE_PUSH = 0x1,
    OO = 0x2,
    OOO = 0x3,
    CAPTURE = 0x8,
    EN_PASSANT = 0xA,
    PROMOTIONS = 0x7,
    PROMOTION_CAPTURES = 0xC,
    PR_KNIGHT = 0x4,
    PR_BISHOP = 0x5,
    PR_ROOK = 0x6,
    PR_QUEEN = 0x7,
    PC_KNIGHT = 0xC,
    PC_BISHOP = 0xD,
    PC_ROOK = 0xE,
    PC_QUEEN = 0xF
} MoveFlag;

typedef struct move {
    MoveFlag flag;
    Square from;
    Square to;
} Move;

// Gets all the moves for the piece on the specified square, adds them to the move list
// and increments the move pointer.
// from: The square the piece is on
// to:   The bitboard with bits set on all the legal moves that piece can make
inline Move* get_moves(Square from, U64 to, Move *list, MoveFlag flag) {
    while (to) {
        Move move = {
            .flag = flag,
            .from = from,
            .to = (Square)pop_lsb(&to)
        };
        *list++ = move;
    }
    return list;
}


void play(Position *pos, Move *m);
void undo(Position *pos, Move *m);

Move* generate_legal_moves(Position *pos, Move *list);

Move parse_move(Position *pos, const char *move_str);

void get_move_str(Move move, char *move_str);

// Checks if a move is legal with atomic rules, and if it is then it adds it to
// the move list. Otherwise, it does nothing.
Move *atomic_add_move(Move *list, Color me, Color you, U64 all_pieces,
                      U64 all_pawns, U64 all_your_pieces,
                      U64 your_orthogonal_sliders, U64 your_diagonal_sliders,
                      U64 your_knights, U64 your_pawns, Square from_sq,
                      Square to_sq, Square my_king_square,
                      int include_promotions, int include_enpassant);

#endif // !MOVEGEN_H
