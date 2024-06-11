#pragma once

#ifndef TABLES_H
#define TABLES_H

#include "utils.h"

extern const U64 KING_ATTACKS[64];
extern const U64 KNIGHT_ATTACKS[64];
extern const U64 WHITE_PAWN_ATTACKS[64];
extern const U64 BLACK_PAWN_ATTACKS[64];

extern U64 SQUARES_BETWEEN[64][64];
extern U64 LINE_BETWEEN[64][64];

void initialize_rook_attacks();
void initialize_bishop_attacks();
void initialize_all_lookups();

U64 get_rook_attacks(int square, U64 occupancies);
U64 get_bishop_attacks(int square, U64 occupancies);

#endif // !TABLES_H
