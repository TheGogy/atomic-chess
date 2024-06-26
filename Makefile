CC = gcc
LIBS = -lm
CFLAGS = -Wall -Wextra -Wpedantic -std=gnu99 -Ofast -flto -march=native -fomit-frame-pointer -foptimize-sibling-calls -fprefetch-loop-arrays -falign-functions $(LIBS)
SRCS = sliders.c bitboards.c tables.c position.c movegen.c perft.c uci.c main.c
OBJS = $(SRCS:.c=.o)
EXEC = atom

.PHONY: all clean install uninstall

all: $(EXEC)

atomic: CFLAGS += -DATOMIC
atomic: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)
