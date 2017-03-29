
#include <vector>
#include "point.h"
#include "board.h"
#include "config.h"

using namespace std;

class Solver {
	public:
		Solver(Config config) : cf(config), width(config.getWidth()), height(config.getHeight()),
			maxDepth(cf.getMaxDepth()), maxBoards(config.getMaxBoards()), 
			cornerValue(config.getCornerValue()), edgeValue(config.getEdgeValue()) {}
		
		// Minimax
		vector<point> getMinimaxMoves(Board board, int player);

		// Scoring
		int evaluateBoard(Board board);
		int evaluateDepthLimitedBoard(Board board);

	protected:
		// Configurations
		Config cf;
		int width;
		int height;
		int maxDepth;
		int maxBoards;
		int cornerValue;
		int edgeValue;
};