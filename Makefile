CC = g++
CFLAGS = -Wall -O2

all: clean othello run

othello: othello.o config.o board.o game.o
	$(CC) $(CFLAGS) -o othello othello.o config.o board.o game.o

run:
	./othello initialbrd.txt evalparams.txt

clean: 
	rm -f *.o *~
