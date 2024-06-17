#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char *argv[]){

  // initialize all lookups (must be called at the start)
  initialize_all_lookups();
  init_zobrist_table();

  Position pos;
  set_from_fen(&pos, "7k/8/8/8/4N3/1K5q/8/8 w - - 0 1");
  print_position(&pos);
  Move move_list[218];
  Move *last = generate_legal_moves(&pos, move_list);

  print_all_moves(&pos, move_list, last);

  return EXIT_SUCCESS;
}
