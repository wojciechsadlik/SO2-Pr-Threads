#pragma once

#include "global.hpp"
#include "Destination.hpp"
#include "Vein.hpp"
#include "Oxygen.hpp"

class Erythrocyte {
	int id{0};
	Coords pos;
	Vein* vein {nullptr};
	unique_ptr<Oxygen> oxygen {nullptr};
	Destination* destination {nullptr};
	int nextDirection {0};
	unique_lock<mutex> entranceLck;
	mutex modifyableMtx;

public:
	Erythrocyte() = default;
	Erythrocyte(int id);
	~Erythrocyte() = default;
	void takeOxygen(unique_ptr<Oxygen> oxygen);
	unique_ptr<Oxygen> giveOxygen();
	bool move();
	void operator()();
	void draw();
	void setVein(Vein* vein);
	Vein* getVein();
	void setPos(Coords pos);
	void setDestination(Destination* destination);
};

Erythrocyte::Erythrocyte(int id): id(id){
}

void Erythrocyte::takeOxygen(unique_ptr<Oxygen> oxygen) {
	lock_guard<mutex> lckm {modifyableMtx};
	this->oxygen = std::move(oxygen);
}

unique_ptr<Oxygen> Erythrocyte::giveOxygen() {
	lock_guard<mutex> lckm {modifyableMtx};
	return std::move(oxygen);
}

bool Erythrocyte::move() {
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

void Erythrocyte::operator()() {
	while (true) {
		unique_lock<mutex> lckb {beatmtx};
		beatcv.wait(lckb);
		lckb.unlock();

		bool veinEnd = move();

		{
			lock_guard<mutex> lckm {modifyableMtx};
			if (nextDirection > 1 && entranceLck) {
					entranceLck.unlock();
			}
		}

		{
			lock_guard<mutex> lcke {endThreadsMtx};
			if (endThreads) break;
		}

		if (veinEnd) destination->interact(*this);
	}

	if (entranceLck) {
		entranceLck.unlock();
	}

	synch_mvwprintw(stdscr, pos.line, pos.col, Color::DEFAULT, "%02d", id);
}

void Erythrocyte::draw() {
	unique_lock<mutex> lckm {modifyableMtx, try_to_lock};
	if (lckm && vein != nullptr) {
		if (oxygen == nullptr)
			synch_mvwprintw(stdscr, pos.line, pos.col, Color::ERYTHROCYTE_NO, "%02d", id);
		else
			synch_mvwprintw(stdscr, pos.line, pos.col, Color::ERYTHROCYTE_O, "%02d", id);
	}
}

Vein* Erythrocyte::getVein() {
	lock_guard<mutex> lcka {modifyableMtx};
	return vein;
}

void Erythrocyte::setVein(Vein* vein) {
	lock_guard<mutex> lckm {modifyableMtx};

	entranceLck = unique_lock<mutex> {vein->entranceMtx};
	this->vein = vein;
	this->pos = vein->getStartPos();
	this->destination = vein->getDestination();
	this->nextDirection = 0;
}

void Erythrocyte::setPos(Coords pos) {
	lock_guard<mutex> lckm {modifyableMtx};
	this->pos = pos;
}

void Erythrocyte::setDestination(Destination* destination) {
	lock_guard<mutex> lckm {modifyableMtx};
	this->destination = destination;
}