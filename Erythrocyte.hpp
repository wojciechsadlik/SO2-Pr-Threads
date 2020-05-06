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

public:
	mutex accessMtx;
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
	this->oxygen = std::move(oxygen);
}

unique_ptr<Oxygen> Erythrocyte::giveOxygen() {
	return std::move(oxygen);
}

bool Erythrocyte::move() {
	bool t = false;
	if (vein != nullptr) {
		char c = '?';
		{
			lock_guard<mutex> lck {vein->dataAccessMtx};
			c = vein->getDirection(nextDirection);
		}

		switch (c) {
			case 'u': --pos.line; break;
			case 'd': ++pos.line; break;
			case 'r': pos.col += 2; break;
			case 'l': pos.col -= 2; break;
			case 'x': t = true; nextDirection = -1;
		}
		++nextDirection;
	}
	
	return t;
}

void Erythrocyte::operator()() {
	while (true) {
		bool t;

		{
			lock_guard<mutex> lcka {accessMtx};
			t = move();
			if (nextDirection >= 2 && entranceLck) {
					entranceLck.unlock();
					entranceLck.release();
			}
		}

		if (t) {
			lock_guard<mutex> lcka {destination->accessMtx};
			destination->interact(*this);
		}

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

void Erythrocyte::draw() {
	if (vein != nullptr) {
		if (oxygen == nullptr)
			synch_mvwprintw(stdscr, pos.line, pos.col, Color::ERYTHROCYTE_NO, "%02d", id);
		else
			synch_mvwprintw(stdscr, pos.line, pos.col, Color::ERYTHROCYTE_O, "%02d", id);
	}
}

Vein* Erythrocyte::getVein() {
	return vein;
}

void Erythrocyte::setVein(Vein* vein) {
	this->vein = vein;

	entranceLck = unique_lock<mutex>{vein->entranceMtx};

	lock_guard<mutex> lck {vein->dataAccessMtx};

	this->pos = vein->getStartPos();
	this->destination = vein->getDestination();
}

void Erythrocyte::setPos(Coords pos) {
	this->pos = pos;
}

void Erythrocyte::setDestination(Destination* destination) {
	this->destination = destination;
}