# Atom

A basic (atomic) chess bot written in pure C. Still in development, expect many bugs!

Current perft score:

Standard, starter position, depth 7:
434123925.225558 nps

## Installation

Requires gcc compiler.

```bash
git clone https://github.com/thegogy/atomic-chess
cd atomic-chess
# For regular version:
make
# For atomic version:
make atomic
```

## Inspiration

Move generation takes elements from [Surge](https://github.com/nkarve/surge) by [nkarve](https://github.com/nkarve), as well as [Gigantua](https://github.com/Gigantua/Gigantua) by [Daniel Inführ](https://github.com/Gigantua), with some parts written from scratch.
