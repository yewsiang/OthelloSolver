#include "config.h"
#include "board.h"

using namespace std;

class Game {
	public:
		Game(Config cf);
		
		void play();
		void nextPlayer();
		void makeMove(char x, char y);

		Board getBoard();

	protected:
		Board board;
};