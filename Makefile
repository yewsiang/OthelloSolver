MPICC?=mpic++

all: othello

othello: *.cpp 
	${MPICC} -std=c++11 -o othellox *.cpp 

run:
	mpirun -np 4 ./othellox initialbrd.txt evalparams.txt

clean: 
	rm othellox
