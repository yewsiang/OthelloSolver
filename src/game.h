#include "config.h"
#include "solver.h"
#include "board.h"
#include "disk.h"

using namespace std;

class Game {
	public:
		Game(Config cf);
		
		void play(string algorithm, int numProcs);
		void switchPlayer();

	protected:
		// Configurations
		int maxDepth;

		// State of game
		Board board;
		Solver solver;
		int currentPlayer;
};