
#include <iomanip>
#include "disk.h"
#include "game.h"

using namespace std;

#define NUM_JOBS_PER_PROC 10

Game::Game(Config cf) : maxDepth(cf.getMaxDepth()), 
						board(cf.getWidth(), cf.getHeight()), 
						solver(cf), 
						currentPlayer(cf.getPlayer()) {
	board.initBoard(cf.getWhiteStartingPositions(), cf.getBlackStartingPositions());
}

void Game::play(string algorithm, int numProcs) {
	
	//while (!board.isGameOver()) {

		// Constantly execute minimax move
		board.printBoard(currentPlayer);

		vector<point> validMoves;
		if (algorithm.compare("PARALLEL_MINIMAX") == 0) {
			validMoves = solver.getParallelMinimaxMoves(board, currentPlayer, maxDepth, numProcs, NUM_JOBS_PER_PROC);

		} else if (algorithm.compare("JOBPOOL_MINIMAX") == 0) {
			validMoves = solver.getJobPoolMinimaxMoves(board, currentPlayer, maxDepth, numProcs, NUM_JOBS_PER_PROC);

		}
		//validMoves = solver.getMinimaxMoves(board, currentPlayer, maxDepth);
		//validMoves = solver.getAlphaBetaMoves(board, currentPlayer, maxDepth);
		
		/*if (board.getNumEmpty() > 8) {
			printf("=========================== PARALLEL ========================\n");
			validMoves = solver.getParallelMinimaxMoves(board, currentPlayer, maxDepth, numProcs);
		} else {
			printf("======================== SEQUENTIAL =========================\n");
			validMoves = solver.getMinimaxMoves(board, currentPlayer, maxDepth);			
		}*/

		//validMoves = solver.getAlphaBetaMoves(board, currentPlayer, maxDepth);
		/*validMoves = solver.getMinimaxMoves(board, currentPlayer, maxDepth);
		if (validMoves.size() == 0) {
			switchPlayer();
		} else {
			point nextMove = validMoves[0];
			board.makeMove(currentPlayer, nextMove.x, nextMove.y);
			switchPlayer();
		}
	}*/

	//printf("\n (HELLO A) \n");

	board.printBoard(currentPlayer);

	//printf("\n (HELLO B) \n");	

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
}

void Game::switchPlayer() {
	currentPlayer = OPP(currentPlayer);
}
