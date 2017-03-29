#include "board.h"

using namespace std;

Board::Board(int width, int height, vector<point> whiteStartingPositions, vector<point> blackStartingPositions) {
	data = new char[8];
}

char getDisk(char x, char y) {
	return 0;
}
void flipDisk(char x, char y) {

}
void setDisk(char player, char x, char y) {

}

bool inRange(char x, char y) {
	return false;
}
bool isValidMove(char player, char x, char y) {
	return false;
}
bool isGameOver() {
	return false;
}

void Board::printBoard() {

}