#include "board.h"

using namespace std;

// Copy constructor
Board::Board(const Board &b): cf(b.cf) {
	width = b.width;
	height = b.height;
	data = new int*[width];

	for (int i = 0; i < width; i++) {
		data[i] = new int[height];

		for (int j = 0; j < height; j++) {
			data[i][j] = b.data[i][j];
		}
	}
}

void Board::initBoard() {
	// Initialize 2D Array
	data = new int*[width];
	for (int i = 0; i < width; i++) {
		data[i] = new int[height];

		// Initialize all to Empty
		for (int j = 0; j < height; j++) {
			data[i][j] = EMPTY;
		}
	}

	// Input White & Black starting positions
	for (point p : whiteStartingPositions) {
		setDisk(WHITE, p.x, p.y);
	}
	for (point p : blackStartingPositions) {
		setDisk(BLACK, p.x, p.y);
	}
}

// Helpers
int Board::getDisk(int x, int y) {
	return data[x][y];
}
void Board::flipDisk(int x, int y) {
	data[x][y] = OPP(data[x][y]);
}
void Board::setDisk(int player, int x, int y) {
	data[x][y] = player;
}
bool Board::inRange(int x, int y) {
	return (x >= 0 && x < width && y >= 0 && y < height);
}

// Checks if player can place disk at (x, y)
bool Board::isValidMove(int player, int x, int y) {
	// Not valid if there is a Disk on it already OR if it is out of board's range
	if (data[x][y] != EMPTY || !inRange(x, y)) {
		return false;
	}
	
	// Go through each direction and test if a Disk can be flipped
	for (int d = 0; d < 8; d++) { 
		int changeX = DIRECTION[d][0];
		int changeY = DIRECTION[d][1];
		int newX    = x + (2 * changeX);
		int newY    = y + (2 * changeY);

		// Disk along the direction has to be opponent's disk AND in board's range
		if (!inRange(newX, newY) || data[x + changeX][y + changeY] != OPP(player)) {
			continue;
		}
		while (inRange(newX, newY)) {
			// If there is your Disk, it means that you can place at current spot
			if (data[newX][newY] == player) {
				return true;
			} 
			// Cannot have Empty spots along the direction
			if (data[newX][newY] == EMPTY) {
				break;
			}
			newX = newX + changeX;
			newY = newY + changeY;
		}
	}
	return false;
}

// Retrieves all valid moves for player
vector<point> Board::getValidMoves(int player) {
	vector<point> validMoves;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			if (isValidMove(player, i, j)) {
				point newPoint = point(i, j);
				validMoves.push_back(newPoint);
			}
		}
	}
	return validMoves;
}

// Places Disk at (x, y) for player & flips opponent Disks. Assume move to be valid.
void Board::makeMove(int player, int x, int y) {
	// Go through each direction and test if a Disk can be flipped
	for (int d = 0; d < 8; d++) { 
		int changeX = DIRECTION[d][0];
		int changeY = DIRECTION[d][1];
		int newX    = x + (2 * changeX);
		int newY    = y + (2 * changeY);

		// Disk along the direction has to be opponent's disk AND in board's range
		if (!inRange(newX, newY) || data[x + changeX][y + changeY] != OPP(player)) {
			continue;
		}
		while (inRange(newX, newY)) {
			// If there is your Disk, reverse and flip the Disks that we've encountered
			if (data[newX][newY] == player) {
				newX -= changeX;
				newY -= changeY;
				while (newX != x || newY != y) {
					flipDisk(newX, newY);
					newX -= changeX;
					newY -= changeY;
				}
				setDisk(player, x, y);
				break;
			} 
			// Cannot have Empty spots along the direction
			if (data[newX][newY] == EMPTY) {
				break;
			}
			newX += changeX;
			newY += changeY;
		}
	}
}

// Returns true if game is over (no more valid moves for both players)
bool Board::isGameOver() {
	bool moveExists = false;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			moveExists = moveExists || isValidMove(WHITE, i, j) || isValidMove(BLACK, i, j);
		}
	}
	return !moveExists;
}

void Board::printBoard(int currentPlayer) {
	// Print Horizontal labels
	string leftMargin = "    "; // For pretty printing
	cout << endl << leftMargin << "     " << ((currentPlayer == BLACK) ? "BLACK (X)" : "WHITE (O)");
	cout << endl << leftMargin << "   ";
	for (int i = 0; i < width; i++) {
		cout << string(1, 'A' + i) << " ";
	}
	cout << endl;

	for (int j = 0; j < height; j++) {
		cout << leftMargin;
		(j < 9) ? cout << (j + 1) << "  " : cout << (j + 1) << " ";
		for (int i = 0; i < width; i++) {
			if (data[i][j] == BLACK) {
				cout << "X";
			} else if (data[i][j] == WHITE) {
				cout << "O";
			} else if (isValidMove(currentPlayer, i, j)) {
				cout << "?";
			} else if (data[i][j] == EMPTY) {
				cout << ".";
			} 
			cout << " ";
		}
		cout << endl;
	}
	cout << endl;
}