#include "game.h"

using namespace std;

Game::Game(Config cf) : board(cf.getWidth(), cf.getHeight(), cf.getWhiteStartingPositions(), cf.getBlackStartingPositions()) {}

void Game::play() {

}
void Game::nextPlayer() {
	
}
void Game::makeMove(char x, char y) {

}

Board Game::getBoard() { return board; } 