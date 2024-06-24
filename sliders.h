#pragma once

#ifndef SLIDERS_H
#define SLIDERS_H
#include "position.h"
#include "bitboards.h"

U64 get_rook_attacks(Square square, U64 occupancies);
U64 get_bishop_attacks(Square square, U64 occupancies);
U64 get_queen_attacks(Square square, U64 occupancies);
U64 get_xray_rook_attacks(Square square, U64 occupancies);
U64 get_xray_bishop_attacks(Square square, U64 occupancies);

#endif // !SLIDERS_H
