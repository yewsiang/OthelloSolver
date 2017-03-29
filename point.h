#ifndef POINT_H
#define POINT_H

#include <string>

using namespace std;

struct point {
	int x;
	int y;

    point(int x, int y):
        x(x), y(y) {}

    point(char x, char y):
        x(x - 'a'), y(y - '0' - 1) {}

    bool operator==(const point& other) const {
        return (other.x == x && other.y == y);
    }

    bool operator!=(const point& other) const {
    	return !(other == *this);
    }

    // X that is on the board (Will be an alphabet)
    string getBoardX() {
    	return string(1, x + 'a');
    }

    // Y that is on the board
    int getBoardY() {
    	return y + 1;
    }

    string toString() {
    	return getBoardX() + to_string(getBoardY());
    }
};

#endif