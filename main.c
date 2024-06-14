#include <stdio.h>
#include <stdlib.h>

#include "bitboards.h"
#include "position.h"
#include "tables.h"

int main(int argc, char *argv[]){

  initialize_all_lookups();

  Position pos;
  set_from_fen(&pos, "r1bk3r/p2pBpNp/n4n2/1p1NP2P/6P1/3P4/P1P1K3/q5b1 w KQkq -");

  print_position(&pos);

  return EXIT_SUCCESS;
}
