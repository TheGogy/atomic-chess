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
  set_from_fen(&pos, "r3k2r/ppp1qppp/2npNn2/2b1p3/2B1P3/3P3P/PPP1QPP1/RNB2RK1 b kq - 0 9");
  print_position(&pos);
  Move move_list[218];
  Move *last = generate_legal_moves(&pos, move_list);

  print_all_moves(move_list, last);

  return EXIT_SUCCESS;
}
