MPICC?=mpic++

all: clean othello run

othello: src/*.cpp 
	${MPICC} -o bin/othello src/*.cpp 

run:
	mpirun -np 4 ./bin/othello config/initialbrd4.txt config/evalparams.txt

clean: 
	#rm bin/othello
