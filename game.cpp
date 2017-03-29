#include "disk.h"
#include "game.h"

using namespace std;

Game::Game(Config config) : cf(config), board(config) {}

void Game::play() {
	board.initBoard();
	board.printBoard(WHITE);
	cout << "Gameover: " << board.isGameOver() << endl << endl;
}
void Game::nextPlayer() {
	
}
void Game::makeMove(char x, char y) {

}

Board Game::getBoard() { return board; } 