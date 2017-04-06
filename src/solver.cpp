
#include "solver.h"

using namespace std;

vector<point> Solver::getBestMoves(Board board, int player, int depth) {
	return getMinimaxMoves(board, player, depth);
}

// Assume game is not over, get the minimax moves
vector<point> Solver::getMinimaxMoves(Board board, int player, int depth) {
	vector<point> validMoves = board.getValidMoves(player);
	if (validMoves.size() == 0) {
		return vector<point>();
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

/*
 * Parallel version of getMinimaxMoves
 *
 * Master will distribute Jobs almost equally amongst Slaves before starting on Jobs itself.
 * After which, Master will wait for CompletedJobs to return from Slaves.
 */
vector<point> Solver::getParallelMinimaxMoves(Board board, int player, int depth, int numProcs) {  	
	vector<point> validMoves = board.getValidMoves(player);
	vector<point> minimaxMoves;
	
	// Initialize jobs
	deque<Job> jobs;
	deque<Board> boards;
	int numResults = validMoves.size();
	int results[numResults];
	for (int i = 0; i < numResults; i++) {
		Board newBoard = board;
		newBoard.makeMove(player, validMoves[i].x, validMoves[i].y);

		int** nb;
		//int** newBoard = board.copyData();
		Job newJob = {i, 0, board.getWidth(), board.getHeight(), OPP(player), depth - 1, nb};
		jobs.push_back(newJob);
		boards.push_back(newBoard);
	}

	minimaxMoves.push_back(validMoves[0]);

	// Send header information of jobs to the rest of the processors
	masterSendJobs(&jobs, &boards, numProcs);

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