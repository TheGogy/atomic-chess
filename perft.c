#include "perft.h"
#include "bitboards.h"
#include "movegen.h"
#include "position.h"
#include <time.h>

#include <stdio.h>

// Perft function, used to measure the speed of the move generator.
// https://www.chessprogramming.org/Perft
U64 perft(Position *pos, unsigned int depth) {
  Move move_list[256];
  U64 n_moves;
  U64 nodes = 0;

  n_moves = generate_legal_moves(pos, move_list) - move_list;

  if (depth <= 1) {
    return n_moves;
  }

  for (int i = 0; i < n_moves; i++) {
    play(pos, &move_list[i]);
    nodes += perft(pos, depth - 1);
    undo(pos, &move_list[i]);
  }
  return nodes;
}

void print_move(Move move) {
  printf("%s%s", SQUARE_TO_STRING[move.from], SQUARE_TO_STRING[move.to]);
  if (move.flag & (PROMOTIONS)) {
    switch (move.flag) {
      case PR_KNIGHT | PC_KNIGHT:
        printf("n");
        break;
      case PR_BISHOP | PC_BISHOP:
        printf("b");
        break;
      case PR_ROOK | PC_ROOK:
        printf("r");
        break;
      case PR_QUEEN | PC_QUEEN:
        printf("r");
        break;
      default:
        break;
    }
  }
  printf(": ");
}

void test_single_perft(const char *fen, int depth, int verbose) {
  Position pos;
  set_from_fen(&pos, fen);
  print_position(&pos);

  printf("Testing up to depth %d\n", depth);
  clock_t start, end;
  double cpu_time_used;
  U64 nodes = 0;

  if (verbose) {
    Move move_list[256];
    U64 n_moves, moves_in_pos;

    start = clock();

    n_moves = generate_legal_moves(&pos, move_list) - move_list;
    for (int i = 0; i < n_moves; i++) {
      play(&pos, &move_list[i]);
      print_move(move_list[i]);
      moves_in_pos = perft(&pos, depth - 1);
      printf("%llu\n", moves_in_pos);
      nodes += moves_in_pos;
      undo(&pos, &move_list[i]);
    }
    end = clock();

  } else {
    start = clock();
    nodes = perft(&pos, depth);
    end = clock();
  }

  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("TOTAL NODES: %llu\n", nodes);
  printf("TIME:        %f\n", cpu_time_used);
  printf("NPS:         %f\n", nodes / cpu_time_used);

}

void test_perft(const char *fen, int depth, U64 expected_nodes) {
  Position pos;
  set_from_fen(&pos, fen);
  U64 nodes = perft(&pos, depth);
  if (nodes == expected_nodes) {
    printf("\033[0;32m[PASS]\033[0;0m %s || Depth: %d\n", fen, depth);
  } else {
    printf("\033[0;31m[FAIL]\033[0;0m %s || Depth: %d || EXPECTED: %llu -- RETURNED: %llu\n",
           fen, depth, expected_nodes, nodes);
  }
}

void run_perft_tests() {
  // Perft tests
  test_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ", 6, 119060324);
  test_perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ", 5, 193690690);
  test_perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 6, 11030083);
  test_perft("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 5, 15833292);
  test_perft("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8 ", 5,  89941194);
  test_perft("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ", 5,   164075551);
  test_perft("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1 ", 6,   179862938);
  test_perft("7k/8/8/p7/1P6/8/8/7K b - - 0 1 ", 6,   41874);
  test_perft("rnbqkb1r/ppppp1pp/7n/4Pp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3 ", 6,   244063299);
}
