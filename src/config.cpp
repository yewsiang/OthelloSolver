#include "config.h"

using namespace std;

Config::Config(char* initialBoardFilename, char* paramsFilename) {
	ifstream initialBoardFile, paramsFile;
	initialBoardFile.open(initialBoardFilename, ios::in);
	paramsFile.open(paramsFilename, ios::in);

	// Read Initial Board
    string line;
    while (getline(initialBoardFile, line)) {
        string token = line.substr(0, line.find(":"));
        string value = line.substr(line.find(" ") + 1);

        if (token.compare("Size") == 0) {
        	string x = value.substr(0, value.find(","));
        	width = stoi(x);

        	string y = value.substr(value.find(",") + 1);
        	height = stoi(y);

        } else if (token.compare("White") == 0) {
        	value = value.substr(value.find("{") + 2, value.find("}") - 2);
        	whiteStartingPositions = extractPoints(value);

        } else if (token.compare("Black") == 0) {
        	value = value.substr(value.find("{") + 2, value.find("}") - 2);
        	blackStartingPositions = extractPoints(value);
        } 
    }

    // Read Evaluation Parameters
    while (getline(paramsFile, line)) {
        string token = line.substr(0, line.find(":"));
        string value = line.substr(line.find(" ") + 1);

        if (token.compare("MaxDepth") == 0) {
        	maxDepth = stoi(value);
        } else if (token.compare("MaxBoards") == 0) {
        	maxBoards = stoi(value);
        } else if (token.compare("CornerValue") == 0) {
        	cornerValue = stoi(value);
        } else if (token.compare("EdgeValue") == 0) {
        	edgeValue = stoi(value);
        } 
    }

    // Print Configurations
    cout << endl << "(Initial Configurations)" << endl;

    cout << "Size: " << getWidth() << "," << getHeight() << endl;
    cout << "White: " << endl;
    cout << "Black: " << endl;

    cout << "MaxDepth: " << getMaxDepth() << endl;
    cout << "MaxBoards: " << getMaxBoards() << endl;
    cout << "CornerValue: " << getCornerValue() << endl;
    cout << "EdgeValue: " << getEdgeValue() << endl;

    cout << "White: ";
    for (point p: getWhiteStartingPositions()) {
		cout << "[" << p.toString() << "]";
	}
	cout << endl << "Black: ";
	for (point p: getBlackStartingPositions()) {
        cout << "[" << p.toString() << "]";
    }
    cout << endl;
}

// Extract the points which are separated by commas
vector<point> Config::extractPoints(string input) {
	string s;
	vector<point> toReturn;
	stringstream inputStream(input);
	
	while(getline(inputStream, s, ',')) {
		point newPoint = point(s[0], s[1]);
		toReturn.push_back(newPoint);
	}
	return toReturn;
}

// Initial Board
int Config::getWidth() { return width; }
int Config::getHeight() { return height; }
vector<point> Config::getWhiteStartingPositions() { return whiteStartingPositions; }
vector<point> Config::getBlackStartingPositions() { return blackStartingPositions; }

// Evaluation Parameters
int Config::getMaxDepth() { return maxDepth; }
int Config::getMaxBoards() { return maxBoards; }
int Config::getCornerValue() { return cornerValue; }
int Config::getEdgeValue() { return edgeValue; }
