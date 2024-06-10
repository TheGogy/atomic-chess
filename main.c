#include <stdio.h>
#include <stdlib.h>

#include "bitboards.h"
#include "tables.h"
#include "utils.h"

int main(int argc, char *argv[]){

  initialize_all_lookups();
  U64 board = 0ULL;
  set_bit(board, e4);

  for (int i = 0; i < 64; i++) {
    print_bitboard(get_rook_attacks(i, board));
    getchar();
  }
  return EXIT_SUCCESS;
}
