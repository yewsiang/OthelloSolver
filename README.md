# Othello Solver
NUS CS3211 Project: Parallel solver for Othello / Reversi

This solver evaluates the best move for a current board position and for a specified player using either the minimax algorithm or minimax with alpha-beta pruning algorithm.
The algorithm, the parallelization method (batch or job pooling) and the other parameters that the solver uses can be customised in the `src/othello.cpp` file.

## Instructions
1) Download OpenMPI (https://www.open-mpi.org/)
2) Run `make` to build the executable `othellox`
3) Run `make run` or <br>
`mpirun -np <NUMBER OF PROCESSORS TO USE> ./bin/othellox config/initialbrd.txt config/evalparams.txt`

## Configurations
1) Initial Board Position (`config/initialbrd.txt`)
* Specify the size of the board
* Specify the current positions of the White and Black disks
* Specify the current Player (Black or White)
* Specify the timeout for the evaluation of each board (Not implemented)

2) Evaluation Parameters (`config/evalparams.txt`)
* Specify the maximum depth of evaluation of the board (`MaxDepth`)
* Specify the maximum number of boards to evaluate (`MaxBoards`) (Not implemented)
* Specify the value given to a corner as heuristic to evaluate the board (`CornerValue`) 
* Specify the value given to an edge as a heuristic to evaluate the board (`EdgeValue`)

3) Algorithm Details (`src/othello.cpp`)
* Specify the algorithm to use (Serial or Parallel Batch or Parallel Job Pool, Minimax or Alpha-beta Pruning)
* Specify the method of job distribution (Random or Sequential)
* Specify the number of jobs that each processor should work on
* Specify the number of jobs to send each processor if algorithm is Job Pooling
