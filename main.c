#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include "bitboards.h"
#include "movegen.h"
#include "position.h"
#include "tables.h"

// Helper function while testing to print all moves in the move list
void print_all_moves(Position *pos, Move *move_list, Move *last){
  Move *current = move_list;
  while (current < last) {
    printf("%s%s || Piece: %c || Flags: %s\n",
           SQUARE_TO_STRING[current->from],
           SQUARE_TO_STRING[current->to],
           PIECE_TO_CHAR[pos->board[current->from]],
           MOVETYPE_TO_STR[current->flags]
    );
    current++;
  }
}

U64 perft(Position *pos, unsigned int depth) {
  Move move_list[256];
  U64 n_moves;
  U64 nodes = 0;

  n_moves = generate_legal_moves(pos, move_list) - move_list;

  if (depth == 1) {
    return n_moves;
  }
  

  for (int i = 0; i < n_moves; i++) {
    // getchar();
    // if (pos->board[move_list[i].from] == WHITE_KING) {
      printf("-----------------------------------------------------\n");
      print_position(pos);
      printf("PLAYING MOVE %s%s || Piece: %c || Flags: %s\n",
             SQUARE_TO_STRING[move_list[i].from],
             SQUARE_TO_STRING[move_list[i].to],
             PIECE_TO_CHAR[pos->board[move_list[i].from]],
             MOVETYPE_TO_STR[move_list[i].flags]
             );
      printf("Depth: %d\n", depth);
      // getchar();
    // };
    play(pos, &move_list[i]);
    nodes += perft(pos, depth - 1);
    printf("UNDOING MOVE %s%s || Piece: %c || Flags: %s\n",
           SQUARE_TO_STRING[move_list[i].from],
           SQUARE_TO_STRING[move_list[i].to],
           PIECE_TO_CHAR[pos->board[move_list[i].from]],
           MOVETYPE_TO_STR[move_list[i].flags]
    );
    undo(pos, &move_list[i]);
  }
  return nodes;
}

int main(int argc, char *argv[]){

  // initialize all lookups (must be called at the start)
  initialize_all_lookups();
  init_zobrist_table();

  Position pos;
  set_from_fen(&pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -");
  // set_from_fen(&pos, "rnbqkbnr/p1pppppp/8/1P6/8/8/1PPPPPPP/RNBQKBNR w KQkq a6");
  print_position(&pos);

  // Move move_list[256];
  // 
  // U64 n_moves = generate_legal_moves(&pos, move_list) - move_list;
  //
  // for (int i = 0; i < n_moves; i++) {
  //   // getchar();
  //   // if (&pos->board[move_list[i].from] == WHITE_KING) {
  //     printf("-----------------------------------------------------\n");
  //     print_position(&pos);
  //     printf("PLAYING MOVE %s%s || Piece: %c || Flags: %s\n",
  //            SQUARE_TO_STRING[move_list[i].from],
  //            SQUARE_TO_STRING[move_list[i].to],
  //            PIECE_TO_CHAR[pos.board[move_list[i].from]],
  //            MOVETYPE_TO_STR[move_list[i].flags]
  //            );
  //     getchar();
  //   // };
  //   play(&pos, &move_list[i]);
  //   printf("UNDOING MOVE %s%s || Piece: %c || Flags: %s\n",
  //          SQUARE_TO_STRING[move_list[i].from],
  //          SQUARE_TO_STRING[move_list[i].to],
  //          PIECE_TO_CHAR[pos.board[move_list[i].from]],
  //          MOVETYPE_TO_STR[move_list[i].flags]
  //   );
  //   undo(&pos, &move_list[i]);
  // }


  // Move *last = generate_legal_moves(&pos, move_list);
  // print_all_moves(&pos, move_list, last);

  clock_t start, end;
  double cpu_time_used;

  start = clock();
  U64 nodes = perft(&pos, 6);
  end = clock();

  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

  printf("\n\nPERFT positions: %llu\n", nodes);
  printf("Time: %f\n", cpu_time_used);
  printf("NPS: %f\n", nodes / cpu_time_used);

  return EXIT_SUCCESS;
}
