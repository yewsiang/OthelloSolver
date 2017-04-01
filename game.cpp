
#include <ctime>
#include <iomanip>
#include "disk.h"
#include "game.h"

using namespace std;

void Game::play() {
	board.initBoard();

	/*board.printBoard(BLACK);

	vector<point> validMoves = solver.getMinimaxMoves(board, BLACK);
	for (point p : validMoves) {
		cout << p.toString() << endl;
	}*/

	switchPlayer();

	// Start timer 
	clock_t begin = clock();

	while (!board.isGameOver()) {
		// Constantly execute minimax move
		board.printBoard(currentPlayer);
		vector<point> validMoves = solver.getMinimaxMoves(board, currentPlayer);
		if (validMoves.size() == 0) {
			switchPlayer();
		} else {
			point nextMove = validMoves[0];
			board.makeMove(currentPlayer, nextMove.x, nextMove.y);
			switchPlayer();
		}
	}
	// End timer
	clock_t end = clock();
  	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	board.printBoard(currentPlayer);

	// Determine who won
	int score = solver.evaluateBoard(board);
	if (score < 0) {
		cout << "Result: WHITE wins" << endl;
	} else if (score > 0) {
		cout << "Result: BLACK wins" << endl;
	} else {
		cout << "Result: DRAW" << endl;
	}
	cout << "Time taken = " << fixed << setprecision(1) << elapsed_secs << " seconds" << endl;
}

void Game::switchPlayer() {
	currentPlayer = OPP(currentPlayer);
}

void Game::makeMove(int x, int y) {
	board.makeMove(currentPlayer, x, y);
	switchPlayer();
	board.printBoard(currentPlayer);
}

point Game::getBestMove() {
	return point(0, 0);
}