
#include "solver.h"

using namespace std;

// Assume game is not over
vector<point> Solver::getMinimaxMoves(Board board, int player) {
	vector<point> validMoves = board.getValidMoves(player);
	if (validMoves.size() == 0) {
		return vector<point>();
	}
	vector<point> minimaxMoves;

	int value = (player == BLACK) ? INT_MIN : INT_MAX;
	for (point validMove : validMoves) {
		Board newBoard = board;
		newBoard.makeMove(player, validMove.x, validMove.y);
		int newValue = (player == BLACK) ? getMinValue(newBoard, OPP(player)) :
										   getMaxValue(newBoard, OPP(player));

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

int Solver::getMinValue(Board board, int player) {
	if (board.isGameOver()) {

		//board.printBoard(player);

		return evaluateBoard(board);
	}
	//cout << "Player: " << ((player == BLACK) ? "BLACK. " : "WHITE. ") << "MIN" << endl;
	//board.printBoard(player);

	vector<point> validMoves = board.getValidMoves(player);
	if (validMoves.size() == 0) {
		return getMaxValue(board, OPP(player));
	}

	int value = INT_MAX;
	for (point validMove : validMoves) {
		Board newBoard = board;
		newBoard.makeMove(player, validMove.x, validMove.y);

		//newBoard.printBoard(OPP(player));

		int newValue = getMaxValue(newBoard, OPP(player));
		value = min(value, newValue);
	}
	return value;
}

int Solver::getMaxValue(Board board, int player) {
	if (board.isGameOver()) {

		//board.printBoard(player);

		return evaluateBoard(board);
	}
	//cout << "Player: " << ((player == BLACK) ? "BLACK. " : "WHITE. ") << "MAX" << endl;
	//board.printBoard(player);

	vector<point> validMoves = board.getValidMoves(player);
	if (validMoves.size() == 0) {
		return getMinValue(board, OPP(player));
	}

	int value = INT_MIN;
	for (point validMove : validMoves) {
		Board newBoard = board;
		newBoard.makeMove(player, validMove.x, validMove.y);

		//newBoard.printBoard(OPP(player));

		int newValue = getMinValue(newBoard, OPP(player));
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