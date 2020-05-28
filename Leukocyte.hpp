#pragma once

#include "global.hpp"
#include "Destination.hpp"
#include "Vein.hpp"
#include "Oxygen.hpp"

class Leukocyte {
	int id{0};
	Coords pos;
	Vein* vein {nullptr};
	Destination* destination {nullptr};
	int nextDirection {0};
	unique_lock<mutex> entranceLck;
	mutex modifyableMtx;

public:
	Leukocyte() = default;
	Leukocyte(int id);
	~Leukocyte() = default;
	bool move();
	void operator()();
	void draw();
	void setVein(Vein* vein);
	Vein* getVein();
	void setPos(Coords pos);
	void setDestination(Destination* destination);
};

Leukocyte::Leukocyte(int id): id(id){
}

bool Leukocyte::move() {
	lock_guard<mutex> lckm {modifyableMtx};
	bool end = false;
	if (vein != nullptr) {
		char c = '?';
		
		c = vein->getDirection(nextDirection);

		switch (c) {
			case 'u': --pos.line; break;
			case 'd': ++pos.line; break;
			case 'r': pos.col += 2; break;
			case 'l': pos.col -= 2; break;
			case 'x': end = true;
		}
		
		if (end) nextDirection = 0;
		else ++nextDirection;
	}
	
	return end;
}

void Leukocyte::operator()() {
	while (true) {
		bool veinEnd = move();
		{
			lock_guard<mutex> lckm {modifyableMtx};
			if (nextDirection >= 2 && entranceLck) {
					entranceLck.unlock();
					entranceLck.release();
			}
		}
		

		if (veinEnd) destination->interact(*this);
		
		this_thread::sleep_for(chrono::milliseconds(500));

		{
			lock_guard<mutex> lcke {endThreadsMtx};
			if (endThreads) break;
		}
	}

	if (entranceLck) {
		entranceLck.unlock();
		entranceLck.release();
	}

	synch_mvwprintw(stdscr, pos.line, pos.col, Color::DEFAULT, "%02d", id);
}

void Leukocyte::draw() {
	unique_lock<mutex> lckm {modifyableMtx, try_to_lock};
	if (lckm && vein != nullptr) {
		synch_mvwprintw(stdscr, pos.line, pos.col, Color::LEUKOCYTE, "%02d", id);
	}
}

Vein* Leukocyte::getVein() {
	lock_guard<mutex> lcka {modifyableMtx};
	return vein;
}

void Leukocyte::setVein(Vein* vein) {
	lock_guard<mutex> lckm {modifyableMtx};

	entranceLck = unique_lock<mutex> {vein->entranceMtx};
	this->vein = vein;
	this->pos = vein->getStartPos();
	this->destination = vein->getDestination();
	this->nextDirection = 0;
}

void Leukocyte::setPos(Coords pos) {
	lock_guard<mutex> lckm {modifyableMtx};
	this->pos = pos;
}

void Leukocyte::setDestination(Destination* destination) {
	lock_guard<mutex> lckm {modifyableMtx};
	this->destination = destination;
}