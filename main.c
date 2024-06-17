#include <stdio.h>
#include <stdlib.h>

#include "bitboards.h"
#include "movegen.h"
#include "position.h"
#include "tables.h"

// Helper function while testing to print all moves in the move list
void print_all_moves(Move *move_list, Move *last){
  Move *current = move_list;
  while (current < last) {
    printf("%s%s - FLAGS %d\n", SQUARE_TO_STRING[current->from], SQUARE_TO_STRING[current->to], current->flags);
    current++;
  }
}

int main(int argc, char *argv[]){

  // initialize all lookups (must be called at the start)
  initialize_all_lookups();
  init_zobrist_table();

  Position pos;
  set_from_fen(&pos, "r1bqk2r/pppp1ppp/2n2n2/2b1p1N1/2B1P3/8/PPPP1PPP/RNBQ1RK1 b kq - 7 5");
  print_position(&pos);
  Move move_list[218];
  Move *last = generate_legal_moves(&pos, move_list);

  print_all_moves(move_list, last);

  return EXIT_SUCCESS;
}
