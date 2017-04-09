MPICC?=mpic++

all: clean othello run

othello: src/*.cpp 
	${MPICC} -std=c++11 -o bin/othello src/*.cpp 

run:
	mpirun -np 4 ./bin/othello config/initialbrd6.txt config/evalparams.txt

clean: 
	#rm bin/othello
