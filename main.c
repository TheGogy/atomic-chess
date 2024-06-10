#include <stdlib.h>

#include "bitboards.h"

int main(int argc, char *argv[]){

  for (int i = 0; i < 65; i++) {
    print_bitboard(SQUARE_BB[i]);
  }
  return EXIT_SUCCESS;
}
