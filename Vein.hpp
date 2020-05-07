#pragma once

#include "global.hpp"
#include "Destination.hpp"

class Vein {
	Coords startPos;
	string directions;
	Destination* destination;
	mutex dataAccessMtx;

public:
	mutex entranceMtx;
	Vein(Coords startPos, string directions);
	~Vein() = default;
	char getDirection(int i);
	Coords getStartPos();
	Coords getEndPos();
	int getId();
	void setDestination(Destination* destination);
	Destination* getDestination();
	void draw();
};

Vein::Vein(Coords startPos, string directions): 
startPos(startPos), directions(directions) {
}

char Vein::getDirection(int i) {
	lock_guard<mutex> lckd {dataAccessMtx};
	if ((size_t) i < directions.size()) return directions[i];
	else return 'x';
}

Coords Vein::getStartPos() {
	lock_guard<mutex> lckd {dataAccessMtx};
	return startPos;
}

Coords Vein::getEndPos() {
	lock_guard<mutex> lckd {dataAccessMtx};
	Coords pos = startPos;
	for (int i = 0; (size_t) i < directions.size(); ++i) {
		switch (directions[i]) {
			case 'u': --pos.line; break;
			case 'd': ++pos.line; break;
			case 'r': pos.col += 2; break;
			case 'l': pos.col -= 2; break;
		}
	}

	return pos;
}

void Vein::setDestination(Destination* destination) {
	lock_guard<mutex> lckd {dataAccessMtx};
	this->destination = destination;
}

Destination* Vein::getDestination() {
	lock_guard<mutex> lckd {dataAccessMtx};
	return destination;
}

void Vein::draw() {
	lock_guard<mutex> lckd {dataAccessMtx};
	int col = startPos.col;
	int line = startPos.line;
	lock_guard<mutex> lckp {printMtx};
	wattron(stdscr, COLOR_PAIR(Color::VEIN));
	for (int i = 0; (size_t) i < directions.size(); ++i) {
		char c = directions[i];
		mvprintw(line, col, " %c", c);
		switch (c) {
			case 'u': --line; break;
			case 'd': ++line; break;
			case 'r': col += 2; break;
			case 'l': col -= 2; break;
		}
	}
	mvprintw(line, col, " x");
	wattroff(stdscr, COLOR_PAIR(Color::VEIN));
}