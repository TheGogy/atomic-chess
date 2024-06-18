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

  printf("Depth: %d\n", depth);
  
  n_moves = generate_legal_moves(pos, move_list) - move_list;

  if (depth == 1) {
    return n_moves;
  }

  for (int i = 0; i < n_moves; i++) {
    play(pos, &move_list[i]);
    print_position(pos);
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
  print_position(&pos);

  clock_t start, end;
  double cpu_time_used;

  start = clock();
  U64 nodes = perft(&pos, 1);
  end = clock();

  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

  printf("\n\nPERFT: %llu\n", nodes);
  printf("Time: %f\n", cpu_time_used);

  return EXIT_SUCCESS;
}
