
#include <vector>
#include "point.h"
#include "board.h"
#include "config.h"

using namespace std;

class Solver {
	public:
		Solver(Config config) : cf(config), width(config.getWidth()), height(config.getHeight()),
			maxDepth(cf.getMaxDepth()), maxBoards(config.getMaxBoards()), 
			cornerValue(config.getCornerValue()), edgeValue(config.getEdgeValue()),
			searchedEntireSpace(true), boardsSearched(0) {}
		
		// Minimax
		vector<point> getMinimaxMoves(Board board, int player, int depth);
		int getMinValue(Board board, int player, int depth);
		int getMaxValue(Board board, int player, int depth);

		// Scoring
		int evaluateBoard(Board board);
		int evaluateDepthLimitedBoard(Board board);

		// Helpers
		bool getSearchedEntireSpace();
		int getBoardsSearched();

	protected:
		// Configurations
		Config cf;
		int width;
		int height;
		int maxDepth;
		int maxBoards;
		int cornerValue;
		int edgeValue;

		// State
		bool searchedEntireSpace;
		int boardsSearched;
};