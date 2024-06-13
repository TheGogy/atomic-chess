#include <stdio.h>
#include <stdlib.h>

#include "bitboards.h"
#include "position.h"
#include "tables.h"

int main(int argc, char *argv[]){

  initialize_all_lookups();

  Position pos;
  set_from_fen(&pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -");

  // printf("NO SQ: %d\n", NO_PIECE);
  print_position(&pos);

  // for (int i = 0; i < 64; i++) {
  //   printf("%d\n", pos.board[i]);
  // }

  return EXIT_SUCCESS;
}
