#include <stdlib.h>

#include "bitboards.h"
#include "position.h"
#include "tables.h"

int main(int argc, char *argv[]){

  initialize_all_lookups();

  Position pos;
  
  for (int i = 0; i < 64; i++) {
    pos.board[i] = NO_PIECE;
  }

  put_piece(&pos, WHITE_KING, a1);

  pos.side_to_play = BLACK;

  Undoinfo history = {
    .enpassant = e3,
    .entry = 0x9000000000000000ULL
  };

  pos.history[0] = history;
  pos.ply = 0;

  print_position(&pos);

  return EXIT_SUCCESS;
}
