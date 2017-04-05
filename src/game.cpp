
#include <ctime>
#include <iomanip>
#include "disk.h"
#include "game.h"

using namespace std;

void Game::play() {
	board.initBoard();

	// TODO: REMOVE
	switchPlayer();

	// Start timer 
	clock_t begin = clock();
	
	while (!board.isGameOver()) {
		// Constantly execute minimax move
		solver.getParallelMinimaxMoves(board, currentPlayer, maxDepth);
		/*
		board.printBoard(currentPlayer);
		vector<point> validMoves = solver.getMinimaxMoves(board, currentPlayer, maxDepth);
		if (validMoves.size() == 0) {
			switchPlayer();
		} else {
			point nextMove = validMoves[0];
			board.makeMove(currentPlayer, nextMove.x, nextMove.y);
			switchPlayer();
		}
		*/
	}

	// End timer
	clock_t end = clock();
  	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	board.printBoard(currentPlayer);

	// Determine who won
	int score = solver.evaluateBoard(board);
	if (score < 0) {
		cout << "Result: WHITE wins. ";
	} else if (score > 0) {
		cout << "Result: BLACK wins. ";
	} else {
		cout << "Result: DRAW. ";
	}
	cout << "Score: " << score << endl;
	cout << "Number of boards assessed: " << solver.getBoardsSearched() << endl;
	cout << "Depth of boards: "  << maxDepth << endl;
	cout << "Entire Space: " << (solver.getSearchedEntireSpace() ? "true" : "false") << endl;
	cout << "Elapsed time in seconds: " << fixed << setprecision(1) << elapsed_secs << endl << endl;
}

void Game::switchPlayer() {
	currentPlayer = OPP(currentPlayer);
}