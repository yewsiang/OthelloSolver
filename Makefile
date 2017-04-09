MPICC?=mpic++

all: clean othello run

othello: src/*.cpp 
	${MPICC} -std=c++11 -o bin/othello src/*.cpp 

run:
	mpirun -np 10 ./bin/othello config/initialbrd.txt config/evalparams.txt

clean: 
	#rm bin/othello
