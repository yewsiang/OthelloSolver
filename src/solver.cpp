
#include "solver.h"

using namespace std;

vector<point> Solver::getBestMoves(Board board, int player, int depth) {
	return getMinimaxMoves(board, player, depth);
}

/***************** PARALLEL ALGORITHMS *****************/

/*
 * Parallel version of getMinimaxMoves
 *
 * Master will distribute Jobs almost equally amongst Slaves before working on Jobs itself.
 * Slaves will execute Minimax on their Jobs.
 */
vector<point> Solver::getParallelMinimaxMoves(Board board, int player, int depth, int numProcs, int numJobsPerProc) {
	printf("======== PARALLEL MINIMAX MOVES ============\n");
	// Timing
	long long startTime = wallClockTime();
	long long before, after;
	long long setupTime = 0; // Time for master to setup all the jobs before sending
	long long commTime = 0;	// Communication
	long long compTime = 0; // Computation
	long long collateResultsTime = 0; // Time for master to collate results of jobs after receiving

	vector<point> validMoves = board.getValidMoves(player);
	if (validMoves.size() == 0) {
		return vector<point>();
	} else if (validMoves.size() == 1) {
		return validMoves;
	}
	for (int i = 0; i <validMoves.size(); i++) {
		cout << "[" << validMoves[i].toString() << "]";
	} cout << endl;

	// Initialize jobs
	deque<Job> jobs;
	deque<Board> boards;
	deque<CompletedJob> waitingJobs;
	before = wallClockTime();
	masterInitialiseJobs(&jobs, &boards, &waitingJobs, validMoves, 
		board, player, depth, maxBoards, cornerValue, edgeValue);

	// Split original Jobs into more Jobs before sending to divide more evenly
	printf("=== Problem Size BEFORE splitting: %lu ===\n", jobs.size());
	splitJobs(&jobs, &boards, &waitingJobs, numProcs, numJobsPerProc);
	after = wallClockTime();
	setupTime += after - before;
	printf("=== Problem Size AFTER splitting: %lu ===\n", jobs.size());
	
	/*for (int i = 0; i < waitingJobs.size(); i++) {
		CompletedJob cj = waitingJobs[i];
		Board currentBoard = boards[i];
		printf("WAITING [ID: %d][PARENT: %d] [%s] [VALUE: %d] [BOARDS: %d]\n", 
			cj.id, cj.parentId, (cj.player == BLACK) ? "BLACK" : "WHITE",
			cj.moveValue, cj.boardsAssessed);
	}*/

	// Send Jobs to Slaves
	before = wallClockTime();
	masterSendBatchJobs(&jobs, &boards, numProcs);
	after = wallClockTime();
	commTime += after - before;

	// Master to work on remaining Jobs
	before = wallClockTime();
	masterWorkOnJobs("BATCH_MINIMAX", &jobs, &boards, &waitingJobs);
	after = wallClockTime();
	compTime += after - before;
	printf(" --- MASTER FINISHED COMPUTATIONAL JOBS: Computation =%6.2f s\n", compTime / 1000000000.0);

	// Collect results from Slaves
	before = wallClockTime();
	masterReceiveCompletedJobs(&waitingJobs, numProcs);
	after = wallClockTime();
	commTime += after - before;

	/*for (int i = 0; i < waitingJobs.size(); i++) {
		CompletedJob cj = waitingJobs[i];
		printf("FINISHED [ID: %d][PARENT: %d] [%s] [VALUE: %d] [BOARDS: %d]\n", 
			cj.id, cj.parentId, (cj.player == BLACK) ? "BLACK" : "WHITE", 
			cj.moveValue, cj.boardsAssessed);
	}*/

	// Combine results from Slave processes
	before = wallClockTime();
	masterRewindMinimaxStack(&waitingJobs);
	after = wallClockTime();
	collateResultsTime += after - before;

	/*for (int i = 0; i < waitingJobs.size(); i++) {
		CompletedJob cj = waitingJobs[i];
		Board currentBoard = boards[i];
		printf("FINISHED REWINDING [ID: %d][PARENT: %d] [%s] [VALUE: %d] [BOARDS: %d]\n", 
			cj.id, cj.parentId, (cj.player == BLACK) ? "BLACK" : "WHITE", 
			cj.moveValue, cj.boardsAssessed);
	}*/

	// Get the best moves
	vector<point> minimaxMoves;
	int bestValue = (player == BLACK) ? INT_MIN : INT_MAX;
	for (int i = 0; i < waitingJobs.size(); i++) {
		point validMove = validMoves[i];
		int newValue = waitingJobs[i].moveValue;
		boardsSearched += waitingJobs[i].boardsAssessed;

		if (player == BLACK && newValue > bestValue) {
			// Clear previous moves
			bestValue = newValue;
			minimaxMoves.clear();
			minimaxMoves.push_back(validMove);

		} else if (player == WHITE && newValue < bestValue) { 
			// Clear previous moves
			bestValue = newValue;
			minimaxMoves.clear();
			minimaxMoves.push_back(validMove);

		} else if (newValue == bestValue) {
			// Add on to a previous move with same value
			minimaxMoves.push_back(validMove);
		}
	}
	printf("Best moves: { ");
	for (int i = 0; i < minimaxMoves.size(); i++) {
		cout << minimaxMoves[i].toString() << " ";
	} printf("}\n");

	after = wallClockTime();
	long long totalTime = after - startTime;
	printf("\n --- MASTER: Setup = %6.2f s, Comm = %6.2f s, Computation = %6.2f s, Collate = %6.2f s\n", 
		setupTime / 1000000000.0, commTime / 1000000000.0, compTime / 1000000000.0, collateResultsTime / 1000000000.0);
	printf("     [TOTAL TIME TAKEN: %6.2f s]\n\n", totalTime / 1000000000.0);

	printf("======== PARALLEL MINIMAX MOVES END ========\n");

	return minimaxMoves;
}

vector<point> Solver::getJobPoolMinimaxMoves(Board board, int player, int depth, int numProcs, int numJobsPerProc) {
	printf("======== JOB POOL MINIMAX MOVES ============\n");
	// Timing
	long long startTime = wallClockTime();
	long long before, after;
	long long setupTime = 0; // Time for master to setup all the jobs before sending
	long long commTime = 0;	// Communication
	long long compTime = 0; // Computation
	long long collateResultsTime = 0; // Time for master to collate results of jobs after receiving

	vector<point> validMoves = board.getValidMoves(player);
	if (validMoves.size() == 0) {
		return vector<point>();
	} else if (validMoves.size() == 1) {
		return validMoves;
	}
	for (int i = 0; i <validMoves.size(); i++) {
		cout << "[" << validMoves[i].toString() << "]";
	} cout << endl;

	// Initialize jobs
	deque<Job> jobs;
	deque<Board> boards;
	deque<CompletedJob> waitingJobs;
	before = wallClockTime();
	masterInitialiseJobs(&jobs, &boards, &waitingJobs, validMoves, 
		board, player, depth, maxBoards, cornerValue, edgeValue);

	// Split original Jobs into more Jobs before sending to divide more evenly
	printf("=== Problem Size BEFORE splitting: %lu ===\n", jobs.size());
	splitJobs(&jobs, &boards, &waitingJobs, numProcs, numJobsPerProc);
	after = wallClockTime();
	setupTime += after - before;
	printf("=== Problem Size AFTER splitting: %lu ===\n", jobs.size());
	
	/*for (int i = 0; i < waitingJobs.size(); i++) {
		CompletedJob cj = waitingJobs[i];
		Board currentBoard = boards[i];
		printf("WAITING [ID: %d][PARENT: %d] [%s] [VALUE: %d] [BOARDS: %d]\n", 
			cj.id, cj.parentId, (cj.player == BLACK) ? "BLACK" : "WHITE",
			cj.moveValue, cj.boardsAssessed);
	}*/

	// Handle Job requests from Slave processes
	int ongoingSlaves = 0;
	int jobSize = 3;
	while(jobs.size() > 0 || ongoingSlaves > 0) {
		//printf("   >>>> jobs.size(): %d, ongoingSlaves: %d", jobs.size(), ongoingSlaves);

		MPI_Status status;
		before = wallClockTime();
		int request;
		MPI_Recv(&request, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		after = wallClockTime();
		commTime += after - before;

		if (request == SLAVE_WANTS_JOBS && jobs.size() > 0) {

			//printf("   >>>> MASTER RECEIVED JOB REQUEST FROM %d\n", status.MPI_SOURCE);

			// If there are Jobs, send those Jobs the Slaves are requesting for them
			before = wallClockTime();
			int response = MASTER_SENDING_JOBS;
			MPI_Send(&response, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
			masterSendJobs(&jobs, &boards, status.MPI_SOURCE, jobSize);
			ongoingSlaves++;
			after = wallClockTime();
			commTime += after - before;

		} else if (request == SLAVE_WANTS_JOBS && jobs.size() <= 0) {

			//printf("   >>>> MASTER RECEIVED JOB REQUEST FROM %d BUT NO JOBS\n", status.MPI_SOURCE);

			// If there are no more Jobs, inform the Slaves so that they will terminate
			before = wallClockTime();
			int response = MASTER_NO_JOBS;
			MPI_Send(&response, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
			after = wallClockTime();
			commTime += after - before;

		} else if (request == SLAVE_SENDING_JOBS) {

			//printf("   >>>> SLAVE %d SENDING COMPLETED JOBS\n", status.MPI_SOURCE);

			// Collect results from Slaves
			before = wallClockTime();
			masterReceiveCompletedJobsFromSlave(&waitingJobs, status.MPI_SOURCE);
			ongoingSlaves--;
			after = wallClockTime();
			commTime += after - before;
		}
	}
	// Tell the last Slave to stop working
	MPI_Status status;
	int request;
	int response = MASTER_NO_JOBS;
	MPI_Recv(&request, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
	MPI_Send(&response, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);

	/*for (int i = 0; i < waitingJobs.size(); i++) {
		CompletedJob cj = waitingJobs[i];
		printf("FINISHED [ID: %d][PARENT: %d] [%s] [VALUE: %d] [BOARDS: %d]\n", 
			cj.id, cj.parentId, (cj.player == BLACK) ? "BLACK" : "WHITE", 
			cj.moveValue, cj.boardsAssessed);
	}*/

	// Combine results from Slave processes
	before = wallClockTime();
	masterRewindMinimaxStack(&waitingJobs);
	after = wallClockTime();
	collateResultsTime += after - before;

	// Get the best moves
	vector<point> minimaxMoves;
	int bestValue = (player == BLACK) ? INT_MIN : INT_MAX;
	for (int i = 0; i < waitingJobs.size(); i++) {
		point validMove = validMoves[i];
		int newValue = waitingJobs[i].moveValue;
		boardsSearched += waitingJobs[i].boardsAssessed;

		if (player == BLACK && newValue > bestValue) {
			// Clear previous moves
			bestValue = newValue;
			minimaxMoves.clear();
			minimaxMoves.push_back(validMove);

		} else if (player == WHITE && newValue < bestValue) { 
			// Clear previous moves
			bestValue = newValue;
			minimaxMoves.clear();
			minimaxMoves.push_back(validMove);

		} else if (newValue == bestValue) {
			// Add on to a previous move with same value
			minimaxMoves.push_back(validMove);
		}
	}
	printf("Best moves: { ");
	for (int i = 0; i < minimaxMoves.size(); i++) {
		cout << minimaxMoves[i].toString() << " ";
	} printf("}\n");

	after = wallClockTime();
	long long totalTime = after - startTime;
	printf("\n --- MASTER: Setup = %6.2f s, Comm = %6.2f s, Computation = %6.2f s, Collate = %6.2f s\n", 
		setupTime / 1000000000.0, commTime / 1000000000.0, compTime / 1000000000.0, collateResultsTime / 1000000000.0);
	printf("     [TOTAL TIME TAKEN: %6.2f s]\n\n", totalTime / 1000000000.0);

	printf("======== JOB POOL MINIMAX MOVES END ========\n");

	return minimaxMoves;
}

/**************** SEQUENTIAL ALGORITHMS ****************/

// Assume game is not over, get the minimax moves
vector<point> Solver::getMinimaxMoves(Board board, int player, int depth) {
	vector<point> validMoves = board.getValidMoves(player);
	if (validMoves.size() == 0) {
		return vector<point>();
	} else if (validMoves.size() == 1) {
		return validMoves;
	}
	vector<point> minimaxMoves;

	int value = (player == BLACK) ? INT_MIN : INT_MAX;
	for (point validMove : validMoves) {
		Board newBoard = board;
		newBoard.makeMove(player, validMove.x, validMove.y);
		int newValue = (player == BLACK) ? getMinValue(newBoard, OPP(player), depth - 1) :
										   getMaxValue(newBoard, OPP(player), depth - 1);

		if (player == BLACK && newValue > value) {
			// Clear previous moves
			value = newValue;
			minimaxMoves.clear();
			minimaxMoves.push_back(validMove);

		} else if (player == WHITE && newValue < value) { 
			// Clear previous moves
			value = newValue;
			minimaxMoves.clear();
			minimaxMoves.push_back(validMove);

		} else if (newValue == value) {
			// Add on to a previous move with same value
			minimaxMoves.push_back(validMove);
		}

		cout << "Current move = " << validMove.toString() << ". Value = " << newValue << endl;
	}
	return minimaxMoves;
}

int Solver::getMinValue(Board board, int player, int depth) {
	// Evaluate boards
	if (board.isGameOver()) {
		return evaluateBoard(board);
	} else if (depth == 0 || boardsSearched >= maxBoards) {
		return evaluateDepthLimitedBoard(board);
	}

	vector<point> validMoves = board.getValidMoves(player);
	if (validMoves.size() == 0) {
		// Skip to next player if no moves
		return getMaxValue(board, OPP(player), depth);
	}

	int value = INT_MAX;
	for (point validMove : validMoves) {
		boardsSearched++;
		Board newBoard = board;
		newBoard.makeMove(player, validMove.x, validMove.y);

		int newValue = getMaxValue(newBoard, OPP(player), depth - 1);
		value = min(value, newValue);
	}
	return value;
}

int Solver::getMaxValue(Board board, int player, int depth) {
	// Evaluate boards
	if (board.isGameOver()) {
		return evaluateBoard(board);
	} else if (depth == 0 || boardsSearched >= maxBoards) {
		return evaluateDepthLimitedBoard(board);
	}

	vector<point> validMoves = board.getValidMoves(player);
	if (validMoves.size() == 0) {
		// Skip to next player if no moves
		return getMinValue(board, OPP(player), depth);
	}

	int value = INT_MIN;
	for (point validMove : validMoves) {
		boardsSearched++;
		Board newBoard = board;
		newBoard.makeMove(player, validMove.x, validMove.y);

		int newValue = getMinValue(newBoard, OPP(player), depth - 1);
		value = max(value, newValue);
	}
	return value;
}

// Minimax algorithm but pruned using alpha-beta pruning
vector<point> Solver::getAlphaBetaMoves(Board board, int player, int depth) {
	vector<point> validMoves = board.getValidMoves(player);
	if (validMoves.size() == 0) {
		return vector<point>();
	} else if (validMoves.size() == 1) {
		return validMoves;
	}
	vector<point> minimaxMoves;

	int value = (player == BLACK) ? INT_MIN : INT_MAX;
	for (point validMove : validMoves) {
		Board newBoard = board;
		newBoard.makeMove(player, validMove.x, validMove.y);
		int newValue = (player == BLACK) ? getAlphaBetaMinValue(INT_MIN, INT_MAX, newBoard, OPP(player), depth - 1)
										 : getAlphaBetaMaxValue(INT_MIN, INT_MAX, newBoard, OPP(player), depth - 1);

		if (player == BLACK && newValue > value) {
			// Clear previous moves
			value = newValue;
			minimaxMoves.clear();
			minimaxMoves.push_back(validMove);

		} else if (player == WHITE && newValue < value) { 
			// Clear previous moves
			value = newValue;
			minimaxMoves.clear();
			minimaxMoves.push_back(validMove);

		} else if (newValue == value) {
			// Add on to a previous move with same value
			minimaxMoves.push_back(validMove);
		}

		cout << "Current move = " << validMove.toString() << ". Value = " << newValue << endl;
	}
	return minimaxMoves;
}

int Solver::getAlphaBetaMinValue(int alpha, int beta, Board board, int player, int depth) {
	// Evaluate boards
	if (board.isGameOver()) {
		return evaluateBoard(board);
	} else if (depth == 0 || boardsSearched >= maxBoards) {
		return evaluateDepthLimitedBoard(board);
	}

	vector<point> validMoves = board.getValidMoves(player);
	if (validMoves.size() == 0) {
		// Skip to next player if no moves
		return getAlphaBetaMaxValue(alpha, beta, board, OPP(player), depth);
	}

	int value = INT_MAX;
	for (point validMove : validMoves) {
		boardsSearched++;
		Board newBoard = board;
		newBoard.makeMove(player, validMove.x, validMove.y);

		int newValue = getAlphaBetaMaxValue(alpha, beta, newBoard, OPP(player), depth - 1);
		value = min(value, newValue);

		// Pruning
		if (value <= alpha) {
			return value;
		}
		beta = min(beta, value);
	}
	return value;
}

int Solver::getAlphaBetaMaxValue(int alpha, int beta, Board board, int player, int depth) {
	// Evaluate boards
	if (board.isGameOver()) {
		return evaluateBoard(board);
	} else if (depth == 0 || boardsSearched >= maxBoards) {
		return evaluateDepthLimitedBoard(board);
	}

	vector<point> validMoves = board.getValidMoves(player);
	if (validMoves.size() == 0) {
		// Skip to next player if no moves
		return getAlphaBetaMinValue(alpha, beta, board, OPP(player), depth);
	}

	int value = INT_MIN;
	for (point validMove : validMoves) {
		boardsSearched++;
		Board newBoard = board;
		newBoard.makeMove(player, validMove.x, validMove.y);

		int newValue = getAlphaBetaMinValue(alpha, beta, newBoard, OPP(player), depth - 1);
		value = max(value, newValue);

		// Pruning
		if (value >= beta) {
			return value;
		}
		alpha = max(alpha, value);
	}
	return value;
}

int Solver::evaluateBoard(Board board) {
	int scoreWhite = 0;
	int scoreBlack = 0;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			if (board.getDisk(i, j) == WHITE) {
				scoreWhite++;
			} else if (board.getDisk(i, j) == BLACK) {
				scoreBlack++;
			}
		}
	}
	return scoreBlack - scoreWhite;
}

int Solver::evaluateDepthLimitedBoard(Board board) {
	// Would not have searched entire space if this evaluation function is used
	searchedEntireSpace = false;

	int scoreWhite = 0;
	int scoreBlack = 0;
	int incrementBy = 1;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			// Increment by different values for different areas of the board
			if ((i == 0 && j == 0) || (i == width - 1 && j == 0) || 
				(i == 0 && j == height - 1) || (i == width - 1 && j == height - 1)) {
				// Corners
				incrementBy = cornerValue;
			} else if (i == 0 || i == width - 1 || j == 0 || j == height - 1) {
				// Edges
				incrementBy = edgeValue;
			} else {
				incrementBy = 1;
			}

			if (board.getDisk(i, j) == WHITE) {
				scoreWhite += incrementBy;
			} else if (board.getDisk(i, j) == BLACK) {
				scoreBlack += incrementBy;
			}
		}
	}
	return scoreBlack - scoreWhite;
}

// Helpers
bool Solver::getSearchedEntireSpace() { return searchedEntireSpace; }
int Solver::getBoardsSearched() { return boardsSearched; }
