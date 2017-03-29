#include "config.h"
#include "solver.h"
#include "board.h"
#include "disk.h"

using namespace std;

class Game {
	public:
		Game(Config config) : cf(config), board(config), solver(config), currentPlayer(BLACK) {}
		
		void play();
		void switchPlayer();
		void makeMove(int x, int y);
		point getBestMove();

	protected:
		// Configurations
		Config cf;

		// State of game
		Board board;
		Solver solver;
		int currentPlayer;
};