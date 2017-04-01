#include <vector>
#include "config.h"
#include "point.h"
#include "disk.h"

#ifndef BOARD_H
#define BOARD_H

const int DIRECTION[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

using namespace std;

class Board {
	public:
		Board(Config config): cf(config), width(cf.getWidth()), height(cf.getHeight()),
			whiteStartingPositions(cf.getWhiteStartingPositions()),
			blackStartingPositions(cf.getBlackStartingPositions()) {}; 
		Board(const Board &b);
		
		// Intialization
		void initBoard();
		
		// Helpers
		int getDisk(int x, int y);
		void flipDisk(int x, int y);
		void setDisk(int player, int x, int y);
		bool inRange(int x, int y);

		// Moves
		bool isValidMove(int player, int x, int y);
		vector<point> getValidMoves(int player);

		void makeMove(int player, int x, int y);
		bool isGameOver();

		// Debugging
		void printBoard(int currentPlayer);

	protected:
		// Configurations
		Config cf;
		int width;
		int height;
		vector<point> whiteStartingPositions;
		vector<point> blackStartingPositions;

		// Disks data
		int** data;
};

#endif