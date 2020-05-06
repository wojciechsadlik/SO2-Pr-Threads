#pragma once

#include "global.hpp"
#include "Destination.hpp"

class Vein {
	int id;
	Coords startPos;
	vector<char> directions;
	Destination* destination;

public:
	mutex entranceMtx;
	mutex dataAccessMtx;
	Vein(int id, Coords startPos, vector<char>& directions);
	~Vein() = default;
	vector<char>::iterator getIterator();
	char getDirection(int i);
	Coords getStartPos();
	int getId();
	void setDestination(Destination* destination);
	Destination* getDestination();
	void draw();
};

Vein::Vein(int id, Coords startPos, vector<char>& directions): 
id(id), startPos(startPos), directions(directions) {
}

vector<char>::iterator Vein::getIterator() {
	return directions.begin();
}

char Vein::getDirection(int i) {
	if ((size_t) i < directions.size()) return directions[i];
	else return 'x';
}

Coords Vein::getStartPos() {
	return startPos;
}

int Vein::getId() {
	return id;
}

void Vein::setDestination(Destination* destination) {
	this->destination = destination;
}

Destination* Vein::getDestination() {
	return destination;
}

void Vein::draw() {
	int col = startPos.col;
	int line = startPos.line;
	lock_guard<mutex> lck {dataAccessMtx};
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