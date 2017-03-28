#include <iostream>
#include "board.h"

Board::Board()
{
	
}

Board::~Board()
{
	
}

void Board::endGame()
{
	std::cout << "ENDING" << std::endl;
}

int Board::addSome(int one, int two)
{
	int sum = one + two;
	std::cout << sum << std::endl;
	return sum;
}