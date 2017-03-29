#include "disk.h"
#include "game.h"

using namespace std;

void Game::play() {
	board.initBoard();

	board.printBoard(currentPlayer);

	cout << "Gameover: " << board.isGameOver() << endl;

	vector<point> validMoves = board.getValidMoves(BLACK);
	for (point p : validMoves) {
		cout << p.toString() << endl;
	}
	cout << "Size: " << validMoves.size() << endl;
	cout << "Score: " << solver.evaluateBoard(board) << endl;
	cout << "DL Score: " << solver.evaluateDepthLimitedBoard(board) << endl;
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