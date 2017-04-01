CC = g++
CFLAGS = -Wall -O2

all: clean othello run

othello: othello.o config.o board.o game.o solver.o
	$(CC) $(CFLAGS) -o othello othello.o config.o board.o game.o solver.o

run:
	./othello config/initialbrd.txt config/evalparams.txt

clean: 
	rm -f *.o *~
