#include <vector>
#include "config.h"
#include "point.h"
#include "disk.h"

const int DIRECTION[8][2] = {{1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}};

using namespace std;

class Board {
	public:
		Board(Config config): cf(config), width(cf.getWidth()), height(cf.getHeight()),
			whiteStartingPositions(cf.getWhiteStartingPositions()),
			blackStartingPositions(cf.getBlackStartingPositions()) {}; 
		
		void initBoard();

		int getDisk(int x, int y);
		void flipDisk(int x, int y);
		void setDisk(int player, int x, int y);

		bool inRange(int x, int y);
		bool isValidMove(int player, int x, int y);
		void makeMove(int player, int x, int y);
		bool isGameOver();

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
