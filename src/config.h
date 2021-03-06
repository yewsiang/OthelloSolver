#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "disk.h"
#include "point.h"

using namespace std;

class Config {
	public:
		Config(char* initialBoardFile, char* paramsFile);

		// Helpers
		vector<point> extractPoints(string input);

		// Initial Board
		int getWidth();
		int getHeight();
		int getPlayer();
		int getTimeout();
		vector<point> getWhiteStartingPositions();
		vector<point> getBlackStartingPositions();

		// Evaluation Parameters
		int getMaxDepth();
		int getMaxBoards();
		int getCornerValue();
		int getEdgeValue();

	protected:
		// Initial Board
		int width;
		int height;
		int player;
		int timeout;
		vector<point> whiteStartingPositions;
		vector<point> blackStartingPositions;

		// Evaluation Parameters
		int maxDepth;
		int maxBoards;
		int cornerValue;
		int edgeValue;
};

#endif