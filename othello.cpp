#include <iostream>

#include "board.h"

int main(int argc, char** argv) {
	std::cout << "Hello world" << std::endl;
	Board* x = new Board();
	x->endGame();
	x->addSome(10, 40);
}