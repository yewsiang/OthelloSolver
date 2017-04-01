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

	while (!board.isGameOver()) {
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