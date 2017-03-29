#include <vector>
#include "point.h"

using namespace std;

class Board {
	public:
		Board(int width, int height, vector<point> whiteStartingPositions, vector<point> blackStartingPositions);
		
		char getDisk(char x, char y);
		void flipDisk(char x, char y);
		void setDisk(char player, char x, char y);

		bool inRange(char x, char y);
		bool isValidMove(char player, char x, char y);
		bool isGameOver();

		void printBoard();

	protected:
		char* data;
};
