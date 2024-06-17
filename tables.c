#include "tables.h"
#include "bitboards.h"
#include <string.h>

// All squares that aren't on the corresponding file(s)
const U64 NOT_A_FILE = 18374403900871474942ULL;
const U64 NOT_H_FILE = 9187201950435737471ULL;
const U64 NOT_GH_FILE = 4557430888798830399ULL;
const U64 NOT_AB_FILE = 18229723555195321596ULL;

const U64 KING_ATTACKS[64] = {
  0x302ULL, 0x705ULL, 0xe0aULL, 0x1c14ULL,
  0x3828ULL, 0x7050ULL, 0xe0a0ULL, 0xc040ULL,
  0x30203ULL, 0x70507ULL, 0xe0a0eULL, 0x1c141cULL,
  0x382838ULL, 0x705070ULL, 0xe0a0e0ULL, 0xc040c0ULL,
  0x3020300ULL, 0x7050700ULL, 0xe0a0e00ULL, 0x1c141c00ULL,
  0x38283800ULL, 0x70507000ULL, 0xe0a0e000ULL, 0xc040c000ULL,
  0x302030000ULL, 0x705070000ULL, 0xe0a0e0000ULL, 0x1c141c0000ULL,
  0x3828380000ULL, 0x7050700000ULL, 0xe0a0e00000ULL, 0xc040c00000ULL,
  0x30203000000ULL, 0x70507000000ULL, 0xe0a0e000000ULL, 0x1c141c000000ULL,
  0x382838000000ULL, 0x705070000000ULL, 0xe0a0e0000000ULL, 0xc040c0000000ULL,
  0x3020300000000ULL, 0x7050700000000ULL, 0xe0a0e00000000ULL, 0x1c141c00000000ULL,
  0x38283800000000ULL, 0x70507000000000ULL, 0xe0a0e000000000ULL, 0xc040c000000000ULL,
  0x302030000000000ULL, 0x705070000000000ULL, 0xe0a0e0000000000ULL, 0x1c141c0000000000ULL,
  0x3828380000000000ULL, 0x7050700000000000ULL, 0xe0a0e00000000000ULL, 0xc040c00000000000ULL,
  0x203000000000000ULL, 0x507000000000000ULL, 0xa0e000000000000ULL, 0x141c000000000000ULL,
  0x2838000000000000ULL, 0x5070000000000000ULL, 0xa0e0000000000000ULL, 0x40c0000000000000ULL,
};

const U64 KNIGHT_ATTACKS[64] = {
  0x20400ULL, 0x50800ULL, 0xa1100ULL, 0x142200ULL,
  0x284400ULL, 0x508800ULL, 0xa01000ULL, 0x402000ULL,
  0x2040004ULL, 0x5080008ULL, 0xa110011ULL, 0x14220022ULL,
  0x28440044ULL, 0x50880088ULL, 0xa0100010ULL, 0x40200020ULL,
  0x204000402ULL, 0x508000805ULL, 0xa1100110aULL, 0x1422002214ULL,
  0x2844004428ULL, 0x5088008850ULL, 0xa0100010a0ULL, 0x4020002040ULL,
  0x20400040200ULL, 0x50800080500ULL, 0xa1100110a00ULL, 0x142200221400ULL,
  0x284400442800ULL, 0x508800885000ULL, 0xa0100010a000ULL, 0x402000204000ULL,
  0x2040004020000ULL, 0x5080008050000ULL, 0xa1100110a0000ULL, 0x14220022140000ULL,
  0x28440044280000ULL, 0x50880088500000ULL, 0xa0100010a00000ULL, 0x40200020400000ULL,
  0x204000402000000ULL, 0x508000805000000ULL, 0xa1100110a000000ULL, 0x1422002214000000ULL,
  0x2844004428000000ULL, 0x5088008850000000ULL, 0xa0100010a0000000ULL, 0x4020002040000000ULL,
  0x400040200000000ULL, 0x800080500000000ULL, 0x1100110a00000000ULL, 0x2200221400000000ULL,
  0x4400442800000000ULL, 0x8800885000000000ULL, 0x100010a000000000ULL, 0x2000204000000000ULL,
  0x4020000000000ULL, 0x8050000000000ULL, 0x110a0000000000ULL, 0x22140000000000ULL,
  0x44280000000000ULL, 0x0088500000000000ULL, 0x0010a00000000000ULL, 0x20400000000 
};

const U64 WHITE_PAWN_ATTACKS[64] = {
  0x200ULL, 0x500ULL, 0xa00ULL, 0x1400ULL,
  0x2800ULL, 0x5000ULL, 0xa000ULL, 0x4000ULL,
  0x20000ULL, 0x50000ULL, 0xa0000ULL, 0x140000ULL,
  0x280000ULL, 0x500000ULL, 0xa00000ULL, 0x400000ULL,
  0x2000000ULL, 0x5000000ULL, 0xa000000ULL, 0x14000000ULL,
  0x28000000ULL, 0x50000000ULL, 0xa0000000ULL, 0x40000000ULL,
  0x200000000ULL, 0x500000000ULL, 0xa00000000ULL, 0x1400000000ULL,
  0x2800000000ULL, 0x5000000000ULL, 0xa000000000ULL, 0x4000000000ULL,
  0x20000000000ULL, 0x50000000000ULL, 0xa0000000000ULL, 0x140000000000ULL,
  0x280000000000ULL, 0x500000000000ULL, 0xa00000000000ULL, 0x400000000000ULL,
  0x2000000000000ULL, 0x5000000000000ULL, 0xa000000000000ULL, 0x14000000000000ULL,
  0x28000000000000ULL, 0x50000000000000ULL, 0xa0000000000000ULL, 0x40000000000000ULL,
  0x200000000000000ULL, 0x500000000000000ULL, 0xa00000000000000ULL, 0x1400000000000000ULL,
  0x2800000000000000ULL, 0x5000000000000000ULL, 0xa000000000000000ULL, 0x4000000000000000ULL,
  0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
  0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
};

const U64 BLACK_PAWN_ATTACKS[64] = {
  0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
  0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
  0x2ULL, 0x5ULL, 0xaULL, 0x14ULL,
  0x28ULL, 0x50ULL, 0xa0ULL, 0x40ULL,
  0x200ULL, 0x500ULL, 0xa00ULL, 0x1400ULL,
  0x2800ULL, 0x5000ULL, 0xa000ULL, 0x4000ULL,
  0x20000ULL, 0x50000ULL, 0xa0000ULL, 0x140000ULL,
  0x280000ULL, 0x500000ULL, 0xa00000ULL, 0x400000ULL,
  0x2000000ULL, 0x5000000ULL, 0xa000000ULL, 0x14000000ULL,
  0x28000000ULL, 0x50000000ULL, 0xa0000000ULL, 0x40000000ULL,
  0x200000000ULL, 0x500000000ULL, 0xa00000000ULL, 0x1400000000ULL,
  0x2800000000ULL, 0x5000000000ULL, 0xa000000000ULL, 0x4000000000ULL,
  0x20000000000ULL, 0x50000000000ULL, 0xa0000000000ULL, 0x140000000000ULL,
  0x280000000000ULL, 0x500000000000ULL, 0xa00000000000ULL, 0x400000000000ULL,
  0x2000000000000ULL, 0x5000000000000ULL, 0xa000000000000ULL, 0x14000000000000ULL,
  0x28000000000000ULL, 0x50000000000000ULL, 0xa0000000000000ULL, 0x40000000000000ULL,

};

const U64 ROOK_MAGICS[64] = {
  0x0080001020400080ULL, 0x0040001000200040ULL, 0x0080081000200080ULL, 0x0080040800100080ULL,
  0x0080020400080080ULL, 0x0080010200040080ULL, 0x0080008001000200ULL, 0x0080002040800100ULL,
  0x0000800020400080ULL, 0x0000400020005000ULL, 0x0000801000200080ULL, 0x0000800800100080ULL,
  0x0000800400080080ULL, 0x0000800200040080ULL, 0x0000800100020080ULL, 0x0000800040800100ULL,
  0x0000208000400080ULL, 0x0000404000201000ULL, 0x0000808010002000ULL, 0x0000808008001000ULL,
  0x0000808004000800ULL, 0x0000808002000400ULL, 0x0000010100020004ULL, 0x0000020000408104ULL,
  0x0000208080004000ULL, 0x0000200040005000ULL, 0x0000100080200080ULL, 0x0000080080100080ULL,
  0x0000040080080080ULL, 0x0000020080040080ULL, 0x0000010080800200ULL, 0x0000800080004100ULL,
  0x0000204000800080ULL, 0x0000200040401000ULL, 0x0000100080802000ULL, 0x0000080080801000ULL,
  0x0000040080800800ULL, 0x0000020080800400ULL, 0x0000020001010004ULL, 0x0000800040800100ULL,
  0x0000204000808000ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
  0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000010002008080ULL, 0x0000004081020004ULL,
  0x0000204000800080ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
  0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000800100020080ULL, 0x0000800041000080ULL,
  0x00FFFCDDFCED714AULL, 0x007FFCDDFCED714AULL, 0x003FFFCDFFD88096ULL, 0x0000040810002101ULL,
  0x0001000204080011ULL, 0x0001000204000801ULL, 0x0001000082000401ULL, 0x0001FFFAABFAD1A2ULL
};

const int ROOK_RELEVANT_BITS[64] = {
  52, 53, 53, 53, 53, 53, 53, 52, 53, 54, 54, 54, 54, 54, 54, 53,
  53, 54, 54, 54, 54, 54, 54, 53, 53, 54, 54, 54, 54, 54, 54, 53,
  53, 54, 54, 54, 54, 54, 54, 53, 53, 54, 54, 54, 54, 54, 54, 53,
  53, 54, 54, 54, 54, 54, 54, 53, 52, 53, 53, 53, 53, 53, 53, 52,
};

const U64 BISHOP_MAGICS[64] = {
  0x0002020202020200ULL, 0x0002020202020000ULL, 0x0004010202000000ULL, 0x0004040080000000ULL,
  0x0001104000000000ULL, 0x0000821040000000ULL, 0x0000410410400000ULL, 0x0000104104104000ULL,
  0x0000040404040400ULL, 0x0000020202020200ULL, 0x0000040102020000ULL, 0x0000040400800000ULL,
  0x0000011040000000ULL, 0x0000008210400000ULL, 0x0000004104104000ULL, 0x0000002082082000ULL,
  0x0004000808080800ULL, 0x0002000404040400ULL, 0x0001000202020200ULL, 0x0000800802004000ULL,
  0x0000800400A00000ULL, 0x0000200100884000ULL, 0x0000400082082000ULL, 0x0000200041041000ULL,
  0x0002080010101000ULL, 0x0001040008080800ULL, 0x0000208004010400ULL, 0x0000404004010200ULL,
  0x0000840000802000ULL, 0x0000404002011000ULL, 0x0000808001041000ULL, 0x0000404000820800ULL,
  0x0001041000202000ULL, 0x0000820800101000ULL, 0x0000104400080800ULL, 0x0000020080080080ULL,
  0x0000404040040100ULL, 0x0000808100020100ULL, 0x0001010100020800ULL, 0x0000808080010400ULL,
  0x0000820820004000ULL, 0x0000410410002000ULL, 0x0000082088001000ULL, 0x0000002011000800ULL,
  0x0000080100400400ULL, 0x0001010101000200ULL, 0x0002020202000400ULL, 0x0001010101000200ULL,
  0x0000410410400000ULL, 0x0000208208200000ULL, 0x0000002084100000ULL, 0x0000000020880000ULL,
  0x0000001002020000ULL, 0x0000040408020000ULL, 0x0004040404040000ULL, 0x0002020202020000ULL,
  0x0000104104104000ULL, 0x0000002082082000ULL, 0x0000000020841000ULL, 0x0000000000208800ULL,
  0x0000000010020200ULL, 0x0000000404080200ULL, 0x0000040404040400ULL, 0x0002020202020200ULL
};

const int BISHOP_RELEVANT_BITS[64] = {
  58, 59, 59, 59, 59, 59, 59, 58, 59, 59, 59, 59, 59, 59, 59, 59,
  59, 59, 57, 57, 57, 57, 59, 59, 59, 59, 57, 55, 55, 57, 59, 59,
  59, 59, 57, 55, 55, 57, 59, 59, 59, 59, 57, 57, 57, 57, 59, 59,
  59, 59, 59, 59, 59, 59, 59, 59, 58, 59, 59, 59, 59, 59, 59, 58,
};

U64 ROOK_MASKS[64];
U64 ROOK_ATTACKS[64][4096];

U64 mask_rook_attacks(Square square) {
  U64 attacks_bitboard = 0ULL; // Attacks bitboard

  int r, f;
  int tr = square / 8;
  int tf = square % 8;

  for (r = tr + 1; r <= 6; r++)
    attacks_bitboard |= (1ULL << (r * 8 + tf));
  for (r = tr - 1; r >= 1; r--)
    attacks_bitboard |= (1ULL << (r * 8 + tf));
  for (f = tf + 1; f <= 6; f++)
    attacks_bitboard |= (1ULL << (tr * 8 + f));
  for (f = tf - 1; f >= 1; f--)
    attacks_bitboard |= (1ULL << (tr * 8 + f));

  return attacks_bitboard;
}

U64 mask_rook_attacks_otf(Square square, U64 block) {
  U64 attacks_bitboard = 0ULL; // Attacks bitboard

  int r, f;
  int tr = square / 8;
  int tf = square % 8;

  for (r = tr + 1; r <= 7; r++) {
    attacks_bitboard |= (1ULL << (r * 8 + tf));
    if ((1ULL << (r * 8 + tf)) & block)
      break;
  }
  for (r = tr - 1; r >= 0; r--) {
    attacks_bitboard |= (1ULL << (r * 8 + tf));
    if ((1ULL << (r * 8 + tf)) & block)
      break;
  }
  for (f = tf + 1; f <= 7; f++) {
    attacks_bitboard |= (1ULL << (tr * 8 + f));
    if ((1ULL << (tr * 8 + f)) & block)
      break;
  }
  for (f = tf - 1; f >= 0; f--) {
    attacks_bitboard |= (1ULL << (tr * 8 + f));
    if ((1ULL << (tr * 8 + f)) & block)
      break;
  }

  return attacks_bitboard;
}

U64 BISHOP_MASKS[64];
U64 BISHOP_ATTACKS[64][4096];

U64 mask_bishop_attacks(Square square) {
  U64 attacks = 0ULL;

  int r, f;
  int tr = square / 8;
  int tf = square % 8;

  for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++)
    attacks |= (1ULL << (r * 8 + f));
  for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++)
    attacks |= (1ULL << (r * 8 + f));
  for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--)
    attacks |= (1ULL << (r * 8 + f));
  for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--)
    attacks |= (1ULL << (r * 8 + f));

  return attacks;
}

U64 mask_bishop_attacks_otf(Square square, U64 block) {
  U64 attacks_bitboard = 0ULL; // Attacks bitboard

  int r, f;
  int tr = square / 8;
  int tf = square % 8;

  for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++) {
    attacks_bitboard |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & block)
      break;
  }
  for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) {
    attacks_bitboard |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & block)
      break;
  }
  for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) {
    attacks_bitboard |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & block)
      break;
  }
  for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
    attacks_bitboard |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & block)
      break;
  }
  return attacks_bitboard;
}

U64 set_occupancies(int index, int bits_in_mask, U64 attack_mask) {
  U64 occupancies = 0ULL;
  for (int count = 0; count < bits_in_mask; count++) {
    Square square = get_lsb_idx(attack_mask);
    pop_bit(attack_mask, square);

    if (index & (1 << count)) {
      // Populate occupancies map
      occupancies |= (1ULL << square);
    }
  }
  return occupancies;
}

void init_rook_attacks() {
  for (Square square = 0; square < 64; square++) {
    ROOK_MASKS[square] = mask_rook_attacks(square);
    U64 attack_mask = ROOK_MASKS[square];
    int relevant_bits_count = count_bits(attack_mask);
    int occupancies_indices = (1 << relevant_bits_count);

    for (int index = 0; index < occupancies_indices; index++) {
      U64 occupancies =
          set_occupancies(index, relevant_bits_count, attack_mask);
      int magic_index =
          (occupancies * ROOK_MAGICS[square]) >> ROOK_RELEVANT_BITS[square];
      ROOK_ATTACKS[square][magic_index] =
          mask_rook_attacks_otf(square, occupancies);
    }
  }
}

void init_bishop_attacks() {
  for (Square square = 0; square < 64; square++) {
    BISHOP_MASKS[square] = mask_bishop_attacks(square);
    U64 attack_mask = BISHOP_MASKS[square];
    int relevant_bits_count = count_bits(attack_mask);
    int occupancies_indices = (1 << relevant_bits_count);

    for (int index = 0; index < occupancies_indices; index++) {
      U64 occupancies =
          set_occupancies(index, relevant_bits_count, attack_mask);
      int magic_index =
          (occupancies * BISHOP_MAGICS[square]) >> BISHOP_RELEVANT_BITS[square];
      BISHOP_ATTACKS[square][magic_index] =
          mask_bishop_attacks_otf(square, occupancies);
    }
  }
}

inline U64 get_rook_attacks(Square square, U64 occupancies) {
  U64 magic_lookup = occupancies & ROOK_MASKS[square];
  magic_lookup *= ROOK_MAGICS[square];
  magic_lookup >>= ROOK_RELEVANT_BITS[square];
  return ROOK_ATTACKS[square][magic_lookup];
}

inline U64 get_xray_rook_lookups(Square square, U64 occupancies) {
  U64 attacks = get_rook_attacks(square, occupancies);
  return attacks ^ get_rook_attacks(square, occupancies ^ (attacks & occupancies));
}

inline U64 get_bishop_attacks(Square square, U64 occupancies) {
  U64 magic_lookup = occupancies & BISHOP_MASKS[square];
  magic_lookup *= BISHOP_MAGICS[square];
  magic_lookup >>= BISHOP_RELEVANT_BITS[square];
  return BISHOP_ATTACKS[square][magic_lookup];
}

inline U64 get_xray_bishop_lookups(Square square, U64 occupancies) {
  U64 attacks = get_bishop_attacks(square, occupancies);
  return attacks ^ get_bishop_attacks(square, occupancies ^ (attacks & occupancies));
}

inline U64 get_queen_attacks(Square square, U64 occupancies) {
  return get_bishop_attacks(square, occupancies) | get_rook_attacks(square, occupancies);
}

U64 SQUARES_BETWEEN[64][64];
U64 LINE_BETWEEN[64][64];
U64 CHECK_BETWEEN[64][64];
U64 PIN_BETWEEN[64][64];

void init_extra_lookups() {
  U64 squares;
  for (int s1 = a8; s1 <= h1; ++s1) {
    int s1_row = s1 / 8;
    int s1_col = s1 % 8;
    int s1_diag = s1_row - s1_col;
    int s1_anti_diag = s1_row + s1_col;

    for (int s2 = a8; s2 <= h1; ++s2) {
      int s2_row = s2 / 8;
      int s2_col = s2 % 8;
      int s2_diag = s2_row - s2_col;
      int s2_anti_diag = s2_row + s2_col;

      squares = SQUARE_TO_BITBOARD[s1] | SQUARE_TO_BITBOARD[s2];

      if (s1_row == s2_row || s1_col == s2_col) {
        // same row or column
        SQUARES_BETWEEN[s1][s2] = mask_rook_attacks_otf(s1, squares) &
                                  mask_rook_attacks_otf(s2, squares);

        LINE_BETWEEN[s1][s2] = (
          mask_rook_attacks_otf(s1, 0ULL) &
          mask_rook_attacks_otf(s2, 0ULL) ) | 
          (SQUARE_TO_BITBOARD[s1] | SQUARE_TO_BITBOARD[s2]);

        CHECK_BETWEEN[s1][s2] = SQUARES_BETWEEN[s1][s2] | (LINE_BETWEEN[s1][s2] & KING_ATTACKS[s1]);

        PIN_BETWEEN[s1][s2] = SQUARES_BETWEEN[s1][s2] | SQUARE_TO_BITBOARD[s2];

      } else if (s1_diag == s2_diag || s1_anti_diag == s2_anti_diag) {
        SQUARES_BETWEEN[s1][s2] = mask_bishop_attacks_otf(s1, squares) &
                                  mask_bishop_attacks_otf(s2, squares);

        LINE_BETWEEN[s1][s2] = (
          mask_bishop_attacks_otf(s1, 0ULL) &
          mask_bishop_attacks_otf(s2, 0ULL) ) | 
          (SQUARE_TO_BITBOARD[s1] | SQUARE_TO_BITBOARD[s2]);

        PIN_BETWEEN[s1][s2] = SQUARES_BETWEEN[s1][s2] | SQUARE_TO_BITBOARD[s2];

      } else {
        SQUARES_BETWEEN[s1][s2] = 0ULL;
        LINE_BETWEEN[s1][s2] = 0ULL;
        CHECK_BETWEEN[s1][s2] = 0ULL;
        PIN_BETWEEN[s1][s2] = 0ULL;
      }
    }
  }
}

U64 PAWN_ATTACKS[2][64];
U64 PSEUDO_LEGAL_ATTACKS[6][64];

void init_pseudo_legal() {
  memcpy(PAWN_ATTACKS[WHITE], WHITE_PAWN_ATTACKS, sizeof(WHITE_PAWN_ATTACKS));
  memcpy(PAWN_ATTACKS[WHITE], WHITE_PAWN_ATTACKS, sizeof(WHITE_PAWN_ATTACKS));
  memcpy(PAWN_ATTACKS[BLACK], BLACK_PAWN_ATTACKS, sizeof(BLACK_PAWN_ATTACKS));
  memcpy(PSEUDO_LEGAL_ATTACKS[KNIGHT], KNIGHT_ATTACKS, sizeof(KNIGHT_ATTACKS));
  memcpy(PSEUDO_LEGAL_ATTACKS[KING], KING_ATTACKS, sizeof(KING_ATTACKS));
  for (Square s = a8; s <= h1; ++s) {
    PSEUDO_LEGAL_ATTACKS[ROOK][s] = mask_rook_attacks(s);
    PSEUDO_LEGAL_ATTACKS[BISHOP][s] = mask_bishop_attacks(s);
    PSEUDO_LEGAL_ATTACKS[QUEEN][s] =
        PSEUDO_LEGAL_ATTACKS[ROOK][s] | PSEUDO_LEGAL_ATTACKS[BISHOP][s];
  }
}

void initialize_all_lookups() {
  init_rook_attacks();
  init_bishop_attacks();
  init_extra_lookups();
  init_pseudo_legal();
}
