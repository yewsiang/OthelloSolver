CC = g++
CFLAGS = -Wall -O2

othello: othello.o board.o
	$(CC) $(CFLAGS) -o othello othello.o board.o
	./othello

clean:
	rm -f *.o *~
