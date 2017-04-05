#include "config.h"
#include "solver.h"
#include "board.h"
#include "disk.h"

using namespace std;

class Game {
	public:
		Game(Config cf) : maxDepth(cf.getMaxDepth()), board(cf), solver(cf), currentPlayer(BLACK) {}
		
		void play(int numProcs);
		void switchPlayer();

	protected:
		// Configurations
		int maxDepth;

		// State of game
		Board board;
		Solver solver;
		int currentPlayer;
};