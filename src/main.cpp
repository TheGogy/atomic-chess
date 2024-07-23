#include "bitboard.h"
#include "uci.h"
#include "zobrist.h"
#include <cstddef>
#include <cstdlib>
#include <ctime>

using namespace Atom;

// Initializes all the lookups that the engine has.
// Should be run as early as possible.
void initEverything() {
    initBBs();
    Zobrist::init();
    srand(time(NULL));
}

int main (int argc, char *argv[]) {
    initEverything();

    Uci uci;
    uci.loop();

    // NNUE::AccumulatorCaches cacheTable = {nnueNets};
    //
    // auto [psqtSmall, positionalSmall] = nnueNets.small.evaluate(pos, &cacheTable.small);
    // auto [psqtBig, positionalBig] = nnueNets.big.evaluate(pos, &cacheTable.big);
    //
    // std::cout << "Piece value static eval:  " << Eval::pieceValueEval(pos) << std::endl;
    // std::cout << "NNUE static eval (small): " << psqtSmall << " (psqt) " << positionalSmall << " (positional)" << std::endl;
    // std::cout << "NNUE static eval (big):   " << psqtBig << " (psqt) " << positionalBig << " (positional)" << std::endl;

    return 0;
}
