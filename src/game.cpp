
#include <ctime>
#include <iomanip>
#include "disk.h"
#include "game.h"

using namespace std;

Game::Game(Config cf) : maxDepth(cf.getMaxDepth()), 
						board(cf.getWidth(), cf.getHeight()), 
						solver(cf), 
						currentPlayer(BLACK) {
	board.initBoard(cf.getWhiteStartingPositions(), cf.getBlackStartingPositions());
}

void Game::play(int numProcs) {
	// TODO: REMOVE
	switchPlayer();

	// Start timer 
	clock_t begin = clock();
	
	int i = 0;
	while (i < 3 && !board.isGameOver()) {
		printf("HELLO WORLD\n\n");
		i++;

		// Constantly execute minimax move
		board.printBoard(currentPlayer);

		vector<point> validMoves = solver.getParallelMinimaxMoves(board, currentPlayer, maxDepth, numProcs);
		//vector<point> validMoves = solver.getMinimaxMoves(board, currentPlayer, maxDepth);
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
