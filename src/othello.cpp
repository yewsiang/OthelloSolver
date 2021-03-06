#include <iostream>
#include <mpi.h>
#include "config.h"
#include "job.h"

using namespace std;

/*************************** PARAMETERS AVAILABLE TO CHANGE *****************************/
/*
 * This is the algorithm that will be used to evalute the best move for the current board.
 * The available options are:
 *
 * 1) SERIAL_MINIMAX: 
 *    Serial version of the minimax algorithm. 
 *
 * 2) SERIAL_ALPHABETA:
 *    Serial version of the minimax with alpha-beta pruning algorithm.
 *
 * 3) BATCH_MINIMAX:
 *    Parallel algorithm where the boards to be evaluated are split into more boards. 
 *    Boards are then sent to Slave processors in as 1 single batch to be evaluated using the
 *    minimax algorithm.
 *
 * 4) BATCH_ALPHABETA:
 *    Boards are then sent to Slave processors in as 1 single batch to be evaluated using the
 *    minimax algorithm with alpha-beta pruning algorithm.
 *
 * 5) JOBPOOL_MINIMAX:
 *    Master maintains a pool of Jobs that have to be evaluated and Slave processors request 
 *    for Jobs to work on. Boards are then sent to Slave processors in small mini-batches to
 *    be evaluated using the minimax algorithm.
 *
 * 6) JOBPOOL_ALPHABETA:
 *    Master maintains a pool of Jobs that have to be evaluated and Slave processors request 
 *    for Jobs to work on. Boards are then sent to Slave processors in small mini-batches to
 *    be evaluated using the minimax alpha-beta pruning algorithm.
 *
 */
string ALGORITHM = "JOBPOOL_ALPHABETA";

/*
 * This is the method that will be used to choose boards to send to the Slave processors.
 * The available options are:
 *
 * Let there be a total of N boards and K boards have to be sent to each Slave processor,
 * 1) SEQUENTIAL:
 *    Sends the first K boards of the total N boards to each Slave processor.
 *
 * 2) RANDOM:
 *    Randomly chooses K boards to send out of the total N boards.
 *
 */
string JOB_DISTRIBUTION = "RANDOM";

/*
 * This is the number of Jobs that each processor should roughly work on. If there less Jobs
 * that this, Master will split the current Jobs into more granular Jobs.
 *
 * This value is capped at 100 to prevent excessive splitting.
 */
#define NUM_JOBS_PER_PROC 1

/*
 * (Only applicable for JOBPOOL_MINIMAX / JOBPOOL_ALPHABETA algorithms)
 * This is the number of boards to send per Job request by Slave processors.
 */
#define JOBPOOL_SEND_SIZE 1
/****************************************************************************************/


int main(int argc, char** argv) {

	MPI_Init(NULL, NULL);
  	int numProcs, id;
  	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
  	MPI_Comm_rank(MPI_COMM_WORLD, &id);

  	if (id == 0) {
		// Retrieve configurations
		Config cf = Config(argv[1], argv[2]);

		// Setup the board
		int maxDepth = cf.getMaxDepth();
		int currentPlayer = cf.getPlayer();
		Board board = Board(cf.getWidth(), cf.getHeight());
		board.initBoard(cf.getWhiteStartingPositions(), cf.getBlackStartingPositions());

		// Initialize solver
		Solver solver = Solver(cf);

		board.printBoard(currentPlayer);

		// Master acts differently depending on algorithm
		vector<point> validMoves;
		/************************* SERIAL *************************/
		if (ALGORITHM.compare("SERIAL_MINIMAX") == 0) {
			validMoves = solver.getMinimaxMoves(board, currentPlayer, maxDepth);

		} else if (ALGORITHM.compare("SERIAL_ALPHABETA") == 0) {
			validMoves = solver.getAlphaBetaMoves(board, currentPlayer, maxDepth);


		/************** SENDING PROBLEMS AS A BATCH ***************/
		} else if (ALGORITHM.compare("BATCH_MINIMAX") == 0 || 
				   ALGORITHM.compare("BATCH_ALPHABETA") == 0) {
			validMoves = solver.getBatchMoves(board, currentPlayer, maxDepth, numProcs, 
				ALGORITHM, JOB_DISTRIBUTION, NUM_JOBS_PER_PROC);


		/********************** JOB POOLING ***********************/ 
		} else if (ALGORITHM.compare("JOBPOOL_MINIMAX") == 0 ||
				   ALGORITHM.compare("JOBPOOL_ALPHABETA") == 0) {
			validMoves = solver.getJobPoolMoves(board, currentPlayer, maxDepth, numProcs, 
				JOB_DISTRIBUTION, NUM_JOBS_PER_PROC, JOBPOOL_SEND_SIZE);
		}

		// Print best moves
		if (validMoves.size() == 0) {
			cout << "Best moves: { na }";
		} else {
			cout << "Best moves: { ";
			for (int i = 0; i < validMoves.size(); i++) {
				cout << validMoves[i].toString() << " ";
			} cout << "}" << endl;
		}

		cout << endl;
		cout << "Number of Processors: " << numProcs << endl;
		cout << "Algorithm: " << ALGORITHM << endl;
	  	cout << "Job distribution: " << JOB_DISTRIBUTION << endl;
	  	cout << "Number of Jobs per Processor: " << NUM_JOBS_PER_PROC << endl;
	  	cout << "Job Pool Send size: " << JOBPOOL_SEND_SIZE << endl;
		cout << "Number of boards assessed: " << solver.getBoardsSearched() << endl << endl;

	} else {

		// Slave process acts differently depending on algorithm
		/************************* SERIAL *************************/
		if (ALGORITHM.compare("SERIAL_MINIMAX") == 0) {
			// Serial Algorithm: Do nothing

		} else if (ALGORITHM.compare("SERIAL_ALPHABETA") == 0) {
			// Serial Algorithm: Do nothing


		/************** SENDING PROBLEMS AS A BATCH ***************/
		} else if (ALGORITHM.compare("BATCH_MINIMAX") == 0 || 
				   ALGORITHM.compare("BATCH_ALPHABETA") == 0) {
			slaveWaitForJob(ALGORITHM, id);


		/********************** JOB POOLING ***********************/
		} else if (ALGORITHM.compare("JOBPOOL_MINIMAX") == 0 ||
				   ALGORITHM.compare("JOBPOOL_ALPHABETA") == 0) {
			slaveRequestJob(ALGORITHM, id);
		} 
	}

	MPI_Finalize();
}