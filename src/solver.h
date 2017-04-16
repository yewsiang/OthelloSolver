
#include <mpi.h>
#include <deque>
#include <vector>
#include <climits>
#include "job.h"
#include "point.h"
#include "board.h"
#include "config.h"

#ifndef SOLVER_H
#define SOLVER_H

using namespace std;

class Solver {
	public:
		Solver(Config cf) : width(cf.getWidth()), height(cf.getHeight()),
			maxDepth(cf.getMaxDepth()), maxBoards(cf.getMaxBoards()), 
			cornerValue(cf.getCornerValue()), edgeValue(cf.getEdgeValue()),
			searchedEntireSpace(true), boardsSearched(0) {}
		Solver(int w, int h, int maxD, int maxB, int cornerV, int edgeV) :
			width(w), height(h), maxDepth(maxD), maxBoards(maxB), 
			cornerValue(cornerV), edgeValue(edgeV), 
			searchedEntireSpace(true), boardsSearched(0) {}
		
		/*************************** PARALLEL ALGORITHMS **************************/		
		vector<point> getBatchMoves(Board board, int player, int depth, int numProcs,
			string algorithm, string jobDistribution, int numJobsPerProc);
		vector<point> getJobPoolMoves(Board board, int player, int depth, int numProcs,
			string jobDistribution, int numJobsPerProc, int jobPoolSendSize);

		/************************** SEQUENTIAL ALGORITHMS **************************/
		// Minimax
		vector<point> getMinimaxMoves(Board board, int player, int depth);
		int getMinValue(Board board, int player, int depth);
		int getMaxValue(Board board, int player, int depth);

		// Minimax with alpha-beta pruning
		vector<point> getAlphaBetaMoves(Board board, int player, int depth);
		int getAlphaBetaMinValue(int alpha, int beta, Board board, int player, int depth);
		int getAlphaBetaMaxValue(int alpha, int beta, Board board, int player, int depth);

		// Scoring
		int evaluateBoard(Board board);
		int evaluateDepthLimitedBoard(Board board);

		// Helpers
		bool getSearchedEntireSpace();
		int getBoardsSearched();

	protected:
		// Configurations
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

#endif
