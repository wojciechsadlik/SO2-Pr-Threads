#pragma once

#include "utils.hpp"
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
	mutex setable;

public:
	Erythrocyte() = default;		// TODO: implement
	~Erythrocyte() = default;;		// TODO: implement
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

// Erythrocyte::Erythrocyte() {
// }

// Erythrocyte::~Erythrocyte() {
// }

void Erythrocyte::takeOxygen(unique_ptr<Oxygen> oxygen) {
	lock_guard<mutex> lcks {setable};
	this->oxygen = std::move(oxygen);
}

unique_ptr<Oxygen> Erythrocyte::giveOxygen() {
	lock_guard<mutex> lcks {setable};
	return std::move(oxygen);
}

bool Erythrocyte::move() {
	bool t = false;
	lock_guard<mutex> lcks {setable};
	if (vein != nullptr) {
		char c = '?';
		{
			lock_guard<mutex> lck {vein->accessMtx};
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
		bool t = move();

		{
			lock_guard<mutex> lcks {setable};
			if (nextDirection >= 2 && entranceLck) {
					entranceLck.unlock();
					entranceLck.release();
			}
		}

		if (t) destination->interact(*this);

		this_thread::sleep_for(chrono::milliseconds(500));
		
		{
			lock_guard<mutex> lck {endThreadsMtx};
			if (endThreads) break;
		}	
	}
	synch_mvwprintw(stdscr, pos.line, pos.col, Color::DEFAULT, "%02d", id);
}

void Erythrocyte::draw() {
	unique_lock<mutex> lcks {setable, try_to_lock};
	if (lcks) {
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
	lock_guard<mutex> lcks {setable};

	this->vein = vein;

	entranceLck = unique_lock<mutex>{vein->entranceMtx};

	lock_guard<mutex> lck {vein->accessMtx};

	this->pos = vein->getStartPos();
	this->destination = vein->getDestination();
}

void Erythrocyte::setPos(Coords pos) {
	lock_guard<mutex> lcks {setable};
	this->pos = pos;
}

void Erythrocyte::setDestination(Destination* destination) {
	lock_guard<mutex> lcks {setable};
	this->destination = destination;
}