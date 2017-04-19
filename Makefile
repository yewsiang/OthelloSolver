MPICC?=mpic++

all: clean othello run

othello: src/*.cpp 
	${MPICC} -std=c++11 -o bin/othellox src/*.cpp 

run:
	mpirun -np 5 ./bin/othellox config/initialbrd10.txt config/evalparams.txt

clean: 
	#rm bin/othello
