MPICC?=mpic++

all: clean othello run

othello: src/*.cpp 
	${MPICC} -o bin/othello src/*.cpp 

run:
	mpirun -np 3 ./bin/othello config/initialbrd.txt config/evalparams.txt

clean: 
	#rm bin/othello
