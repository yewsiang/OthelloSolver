#include <iostream>
#include "config.h"
#include "game.h"

using namespace std;

int main(int argc, char** argv) {

	// Retrieve configurations
	Config cf = Config(argv[1], argv[2]);

	// Setup the game
	Game game = Game(cf);

}