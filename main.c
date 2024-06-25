#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "position.h"
#include "tables.h"
#include "perft.h"
#include "uci.h"

// Commnd line options
const struct option long_options[] = {
  {"help",      no_argument,       0,  'h' },
  {"run-tests", no_argument,       0,  't' },
  {"perft",     required_argument, 0,  'p' },
  {0,           0,                 0,  0   }
};

void print_help() {
  printf(
    "Usage: [options] [command]\n"
    "Options:\n"
    "-v                         Verbose: prints more info about command\n"
    "Command:\n"
    "-h, --help:                Prints this menu.\n"
    "-t, --run-tests:           Runs a series of perft tests to ensure correct move generation.\n"
    "-p, --perft [fen] [depth]  Runs a perft test on the given fen up to the given depth.\n"
  );
}

int main(int argc, char *argv[]) {

  // initialize all lookups (must be called at the start)
  init_pin_between();
  init_zobrist_table();

  // If there are options, parse them.
  // Otherwise, go through to UCI interface.
  if (argc > 1) {
    int opt;
    int option_index = 0;
    int verbose = 0;
    while ((opt = getopt_long(argc, argv, "vhtp", long_options, &option_index)) != 0) {
      switch (opt) {
        case 'v':
          verbose = 1;
          break;
        case 'h':
          print_help();
          exit(EXIT_SUCCESS);
        case 't':
          run_perft_tests();
          exit(EXIT_SUCCESS);
        case 'p':
          if (optind < argc - 1) {
            char *fen = argv[optind];
            int depth = atoi(argv[optind + 1]);
            test_single_perft(fen, depth, verbose);
            // Move index past the fen + depth
            optind += 2;
            exit(EXIT_SUCCESS);
          } else {
            fprintf(stderr, "Option --perft requires two arguments: <fen> <depth>\n");
            exit(EXIT_FAILURE);
          }
        default:
          print_help();
          exit(EXIT_FAILURE);
      }
    }
  }

  handle_uci();

  return EXIT_SUCCESS;
}
