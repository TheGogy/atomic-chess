#pragma once

#ifndef SLIDERS_H
#define SLIDERS_H
#include "position.h"
#include "bitboards.h"

U64 perft(Position *pos, unsigned int depth);
void test_single_perft(const char *fen, int depth);
void test_perft(const char *fen, int depth, U64 expected_nodes);
void run_perft_tests();

#endif // !SLIDERS_H
