#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "movegen.h"
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
        "-h, --help                 Prints this menu.\n"
        "-t, --run-tests [path]     Runs a series of perft tests to ensure correct move generation.\n"
        "-p, --perft [fen] [depth]  Runs a perft test on the given fen up to the given depth.\n"
    );
}

int main(int argc, char *argv[]) {

    // initialize all lookups (must be called at the start)
    init_pin_between();
    init_zobrist_table();

    // Position pos;
    // set_from_fen(&pos, "1k1r4/3r4/3r4/8/8/3R4/3K4/8 w - - 0 1");
    // print_position(&pos);
    //
    // Move move_list[256];
    // U64 n_moves = generate_legal_moves(&pos, move_list) - move_list;
    // for (int i = 0; i < n_moves; i++) {
    //     getchar();
    //     play(&pos, &move_list[i]);
    //     print_position(&pos);
    //     getchar();
    //     undo(&pos, &move_list[i]);
    //     print_position(&pos);
    // }
    //
    // return EXIT_SUCCESS;


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
                    if (optind < argc) {
                        const char *filename = argv[optind];
                        test_perft_from_file(filename);
                        exit(EXIT_SUCCESS);
                    } else {
                        fprintf(stderr, "Option --test requires an argument: <path>\n");
                        exit(EXIT_FAILURE);
                    }

                case 'p':
                    if (optind < argc - 1) {
                        char *fen = argv[optind];
                        int depth = atoi(argv[optind + 1]);
                        test_single_perft(fen, depth, verbose);
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
