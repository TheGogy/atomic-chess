#pragma once

#ifndef SLIDERS_H
#define SLIDERS_H
#include "position.h"
#include "bitboards.h"

U64 perft(Position *pos, unsigned int depth);
void test_single_perft(const char *fen, int depth, int verbose);
int test_perft(const char *fen, int depth, U64 expected_nodes);
void test_perft_from_file(const char *filename);

#endif // !SLIDERS_H
