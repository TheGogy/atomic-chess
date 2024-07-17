# Atom

A basic chess bot written in C++. Still in development, expect many bugs!

Current perft score (i5-11400H @ 2.7GHz, max 4.5GHz):
- starting position (depth 7): 3195901860 nodes, 4250 ms, 751976908 nps
- kiwipete position (depth 7): 374190009323 nodes, 274811 ms, 1361626751 nps

## Installation

Requires g++ compiler.
```bash
git clone https://github.com/thegogy/atom
cd atom
make release
```

## Inspiration

Move generation takes a lot of inspiration from [VincentBab](https://github.com/vincentbab)'s [Belette](https://github.com/vincentbab/Belette/), as well as [Daniel inführ](https://github.com/Gigantua)'s [Gigantua](https://www.codeproject.com/Articles/5313417/Worlds-fastest-Bitboard-Chess-Movegenerator), as well as many techniques from the [Chess Programming Wiki](https://www.chessprogramming.org/Move_Generation).

Perft tests from [Elcabesa](https://github.com/elcabesa)'s engine, [Vajolet](https://github.com/elcabesa/vajolet).
