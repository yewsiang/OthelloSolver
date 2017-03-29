#include "config.h"
#include "board.h"
#include "disk.h"

using namespace std;

class Game {
	public:
		Game(Config cf);
		
		void play();
		void nextPlayer();
		void makeMove(char x, char y);

		Board getBoard();

	protected:
		Config cf;
		Board board;
};