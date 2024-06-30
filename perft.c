#include "perft.h"
#include "bitboards.h"
#include "movegen.h"
#include "position.h"
#include <ctype.h>
#include <string.h>
#include <time.h>

#include <stdio.h>

// Perft function, used to measure the speed of the move generator.
// https://www.chessprogramming.org/Perft
U64 perft(Position *pos, unsigned int depth) {
    Move move_list[256];
    U64 n_moves;
    U64 nodes = 0;

    n_moves = generate_legal_moves(pos, move_list) - move_list;

    if (depth <= 1) {
        return n_moves;
    }

    for (int i = 0; i < n_moves; i++) {
        play(pos, &move_list[i]);
        nodes += perft(pos, depth - 1);
        undo(pos, &move_list[i]);
    }
    return nodes;
}

// Tests a single perft value. Good for debugging purposes.
void test_single_perft(const char *fen, int depth, int verbose) {
    Position pos;
    set_from_fen(&pos, fen);
    print_position(&pos);

    printf("Testing up to depth %d\n", depth);
    clock_t start, end;
    double cpu_time_used;
    U64 nodes = 0;

    if (verbose) {
        Move move_list[256];
        U64 n_moves, moves_in_pos;

        start = clock();

        n_moves = generate_legal_moves(&pos, move_list) - move_list;
        for (int i = 0; i < n_moves; i++) {
            play(&pos, &move_list[i]);
            moves_in_pos = perft(&pos, depth - 1);
            char move_str[6];
            get_move_str(move_list[i], move_str);
            printf("%s: %llu\n", move_str, moves_in_pos);
            nodes += moves_in_pos;
            undo(&pos, &move_list[i]);
        }
        end = clock();

    } else {
        start = clock();
        nodes = perft(&pos, depth);
        end = clock();
    }

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("TOTAL NODES: %llu\n", nodes);
    printf("TIME:        %f\n", cpu_time_used);
    printf("NPS:         %f\n", nodes / cpu_time_used);

}

// Runs a perft test according to a pre defined number of nodes.
// Returns 1 if test passes, 0 if it fails.
int run_test(const char *fen, int depth, U64 expected_nodes) {
    Position pos;
    set_from_fen(&pos, fen);
    U64 nodes = perft(&pos, depth);
    if (nodes == expected_nodes) {
        printf("\033[0;32m[PASS]\033[0;0m %s || Depth: %d\n", fen, depth);
        return 1;
    } else {
        printf("\033[0;31m[FAIL]\033[0;0m %s || Depth: %d || EXPECTED: %llu -- RETURNED: %llu\n",
               fen, depth, expected_nodes, nodes);
        return 0;
    }
}

// Reads a perft test file and tests each of the values.
void test_perft_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    int tests_passed = 0;
    int total_tests = 0;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character from the end of the line
        line[strcspn(line, "\n")] = '\0';

        // Split the line into FEN and perft values
        char *fen = strtok(line, ";");

        // If the line is empty, continue
        if (!fen) continue;

        // Print divider
        printf("\n################################################################\n\n");

        char *token;
        while ((token = strtok(NULL, ";")) != NULL) {
            // Skip leading spaces
            while (*token == ' ') token++;

            // Check if the token starts with 'D' indicating a depth value
            if (token[0] == 'D' && isdigit(token[1])) {
                int depth;
                U64 expected_nodes;
                if (sscanf(token, "D%d %llu", &depth, &expected_nodes) == 2) {
                    // Test the given perft and update counters
                    tests_passed += run_test(fen, depth, expected_nodes);
                    total_tests++;
                }
            }
        }
    }
    fclose(file);

    // Show results
    printf("\n\n");
    printf("Perft results for %s\n", filename);
    printf("Total tests:  %d\n", total_tests);
    printf("Tests passed: %d\n", tests_passed);
}
