
#include "uci.h"

namespace Atom {

// Formats the given square according to the UCI standard (a1 --> h8)
std::string Uci::formatSquare(Square sq) {
    std::string str;
    str += 'a' + fileOf(sq);
    str += '1' + rankOf(sq);
    return str;
}


// Parses the given square according to UCI standard (a1 --> h8)
Square Uci::parseSquare(std::string str) {
    if (str.length() < 2) return SQ_NONE;

    char col = str[0], row = str[1];
    if ( col < 'a' || col > 'h') return SQ_NONE;
    if ( row < '1' || row > '8') return SQ_NONE;

    return createSquare(File(col - 'a'), Rank(row - '1'));
}


// Formats the given move according to UCI standard.
// This is: <square from><square to><promotion piece>
// e.g "e2e4"
// or  "e7e8q"
//
// Null moves are returned as "0000", although this should
// not happen in normal play.
std::string Uci::formatMove(Move m) {
    if (m == MOVE_NONE) {
        return "(none)";
    } else if (m == MOVE_NULL) {
        return "0000";
    }

    std::string str = formatSquare(moveFrom(m)) + formatSquare(moveTo(m));

    if (moveType(m) == PROMOTION) {
        str += "?pnbrq?"[movePromotionType(m)];
    }

    return str;
}

} // namespace Atom

