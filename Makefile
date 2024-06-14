
CC = gcc
LIBS = -lm
CFLAGS = -Wall -Wextra -Wpedantic -std=gnu99 -Ofast $(LIBS)
SRCS = position.c tables.c bitboards.c movegen.c main.c
OBJS = $(SRCS:.c=.o)
EXEC = atom

.PHONY: all clean install uninstall

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)
