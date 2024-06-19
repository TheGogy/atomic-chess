#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include "bitboards.h"
#include "movegen.h"
#include "position.h"
#include "tables.h"

// Helper function while testing to print all moves in the move list
void print_all_moves(Position *pos, Move *move_list, Move *last){
  Move *current = move_list;
  while (current < last) {
    printf("%s%s || Piece: %c || Flags: %s\n",
           SQUARE_TO_STRING[current->from],
           SQUARE_TO_STRING[current->to],
           PIECE_TO_CHAR[pos->board[current->from]],
           MOVETYPE_TO_STR[current->flags]
    );
    current++;
  }
}

U64 perft(Position *pos, unsigned int depth) {
  Move move_list[256];
  U64 n_moves;
  U64 nodes = 0;

  n_moves = generate_legal_moves(pos, move_list) - move_list;

  if (depth == 1) {
    return n_moves;
  }

  print_position(pos);

  for (int i = 0; i < n_moves; i++) {
    printf("%s%s || Piece: %c || Flags: %s\n",
           SQUARE_TO_STRING[move_list[i].from],
           SQUARE_TO_STRING[move_list[i].to],
           PIECE_TO_CHAR[pos->board[move_list[i].from]],
           MOVETYPE_TO_STR[move_list[i].flags]
    );
    play(pos, &move_list[i]);
    nodes += perft(pos, depth - 1);
    undo(pos, &move_list[i]);
  }
  return nodes;
}

int main(int argc, char *argv[]){

  // initialize all lookups (must be called at the start)
  initialize_all_lookups();
  init_zobrist_table();

  Position pos;
  set_from_fen(&pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -");
  // set_from_fen(&pos, "8/1q6/8/3pP3/8/5K2/8/8 w - d6 0 1");
  print_position(&pos);

  Move move_list[256];
  Move *last = generate_legal_moves(&pos, move_list);
  print_all_moves(&pos, move_list, last);

  clock_t start, end;
  double cpu_time_used;

  start = clock();
  U64 nodes = perft(&pos, 6);
  end = clock();

  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

  printf("\n\nPERFT positions: %llu\n", nodes);
  printf("Time: %f\n", cpu_time_used);

  return EXIT_SUCCESS;
}
