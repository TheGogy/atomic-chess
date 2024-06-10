#ifndef UTILS_H
#define U64 unsigned long long

// Bitboard utilities
#define get_bit(bitboard, square) ((bitboard) &   (1ULL << (square)))
#define set_bit(bitboard, square) ((bitboard) |=  (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

#define count_bits(bitboard) __builtin_popcountll(bitboard)
#define get_lsb_idx(bitboard) (bitboard ? __builtin_ctzll(bitboard) : -1)


#endif // !UTILS_H
