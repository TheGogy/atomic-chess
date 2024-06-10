#include <stdlib.h>

#include "bitboards.h"
#include "tables.h"

int main(int argc, char *argv[]){

  for (int i = 0; i < 64; i++) {
    print_bitboard(BLACK_PAWN_ATTACKS[i]);
  }
  return EXIT_SUCCESS;
}
