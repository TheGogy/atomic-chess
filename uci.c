#include "uci.h"
#include "bitboards.h"
#include "movegen.h"
#include "position.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void uci_position(Position *pos, const char *input) {
  // Run over the position multiple times (there may be multiple commands in there)
  // printf("INPUT '%s'\n", input);
  if (strncmp(input, "position startpos", 17) == 0) {
    set_from_fen(pos, STARTPOS);
  } else if (strncmp(input, "position fen", 12) == 0) {
    const char *fen = input + 13;
    set_from_fen(pos, fen);
  }

  // This needs to be separate: we might have moves outside the regular FEN
  if (strstr(input, "moves")) {
    // UCI protocol states that we can assume that we are in a valid
    // position before any moves are made: pos will have been initialized
    const char *moves = strstr(input, "moves") + 6;
    while (*moves) {
      char move_str[MAX_MOVE_STR_LEN];
      sscanf(moves, "%4s", move_str);
      Move move = parse_move(pos, move_str);

      if (move.from != NO_SQUARE) {
        play(pos, &move);
      } else {
        printf("info string Illegal move: %s\n", move_str);
      }

      // Increment the move counter to go to the next move
      moves += strlen(move_str) + 1;
    }
  }
}

void uci_go(Position *pos) {
  // Placeholder: Choose a random legal move and return it
  Move move_list[256];
  U64 n_moves;

  n_moves = generate_legal_moves(pos, move_list) - move_list;
  if (n_moves > 0) {
    Move best_move = move_list[rand() % n_moves];
    char move_str[MAX_MOVE_STR_LEN];
    get_move_str(best_move, move_str);
    printf("info depth 1\n");
    printf("bestmove %s\n", move_str);
  } else {
    // Null move. This should not happen in normal play.
    printf("0000\n");
  }
}

void handle_uci() {

  // Turn off buffering for input / output
  setbuf(stdout, NULL);
  setbuf(stdin, NULL);

  // Get our position (this will be constant throughout the UCI interaction)
  Position pos;

  // Input buffer
  char input[2000];

  FILE *fptr;
  srand(time(NULL));

  // Main UCI loop
  while (fgets(input, sizeof(input), stdin)) {

    if (strncmp(input, "uci", 3) == 0) {
      printf("id name %s\n", ENGINE_NAME);
      printf("id author %s\n", ENGINE_AUTHOR);
      printf("uciok\n");
    } else if (strncmp(input, "isready", 7) == 0) {
      printf("readyok\n");
    } else if (strncmp(input, "position", 8) == 0) {
      uci_position(&pos, input);
    } else if (strncmp(input, "go", 2) == 0) {
      uci_go(&pos);
    } else if (strncmp(input, "quit", 4) == 0) {
      break;
    }
  }
}

