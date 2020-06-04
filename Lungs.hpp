#pragma once

#include "global.hpp"
#include "Destination.hpp"
#include "Vein.hpp"
#include "Oxygen.hpp"
#include "Erythrocyte.hpp"
#include "Leukocyte.hpp"

class Lungs : public Destination{
	Coords pos;
	const int WIN_LINES {6};
	const int WIN_COLS {17};
	WINDOW* win {nullptr};
	list<unique_ptr<Oxygen>> oxygen;
	mutex oxygenMtx;
	size_t capacity {15};	// maksymalna liczba jednostek tlenu
	Vein* vIn {nullptr};
	Vein* vOut {nullptr};

public:
	Lungs(Coords pos);
	~Lungs();
	void inhale();
	void exhale();
	void operator()();
	void drawOxygen();
	void refresh();
	void interact(Erythrocyte& erythrocyte);
	void interact(Leukocyte& Leukocyte);
	Coords vOutPos();
	void setVeins(Vein* vIn, Vein* vOut);
};

Lungs::Lungs(Coords pos): pos(pos) {
	win = newwin(WIN_LINES, WIN_COLS, pos.line, pos.col);
	box(win, 0, 0);
	mvwprintw(win, 0, 0, "Lungs");
}

Lungs::~Lungs() {
	delwin(win);
}

void Lungs::operator()() {
	while (true) {
		inhale();
		exhale();

		lock_guard<mutex> lck {endThreadsMtx};
		if (endThreads) break;
	}
	synch_wClearLine(win, 1, 1, WIN_COLS - 1);
	synch_wClearLine(win, 2, 1, WIN_COLS - 1);
	synch_mvwprintw(win, 1, 1, Color::DEFAULT, "ended");
}

void Lungs::inhale() {
	synch_wClearLine(win, 1, 1, WIN_COLS - 1);
	synch_wClearLine(win, 2, 1, WIN_COLS - 1);
	synch_mvwprintw(win, 1, 1, Color::INHALE, "inhale");
	chrono::milliseconds taskTime {randomTime(TASK_TIME_LB, TASK_TIME_UB)};		//czas snu - [2.5s, 3.5s]
	for (int i = 1; i < WIN_COLS - 1; ++i) {					//pasek postepu
		{
			lock_guard<mutex> lck {oxygenMtx};
			if (oxygen.size() < capacity)
				oxygen.push_back(unique_ptr<Oxygen> {new Oxygen()});
		}
		synch_mvwaddch(win, 2, i, '*');
		this_thread::sleep_for(taskTime / WIN_COLS);
	}
}

void Lungs::exhale() {
	synch_wClearLine(win, 1, 1, WIN_COLS - 1);
	synch_wClearLine(win, 2, 1, WIN_COLS - 1);
	synch_mvwprintw(win, 1, 1, Color::EXHALE, "exhale");
	chrono::milliseconds taskTime {randomTime(TASK_TIME_LB, TASK_TIME_UB)};		//czas snu - [2.5s, 3.5s]
	for (int i = 1; i < WIN_COLS - 1; ++i) {					//pasek postepu
		{
			lock_guard<mutex> lck {oxygenMtx};
			if (oxygen.size() > 1)
				oxygen.pop_back();
		}
		synch_mvwaddch(win, 2, i, '*');
		this_thread::sleep_for(taskTime / WIN_COLS);
	}
}

void Lungs::drawOxygen() {
	synch_wClearLine(win, 3, 1, WIN_COLS - 1);
	synch_wClearLine(win, 4, 1, WIN_COLS - 1);

	int size = 0;
	{
		lock_guard<mutex> lck {oxygenMtx};
		size = oxygen.size();
	}

	for (int i = 0; i < size; ++i) {
		synch_mvwaddch(win, 3 + i / (WIN_COLS - 2), 1 + i % (WIN_COLS - 2), Color::OXYGEN, ' ');
	}
}

void Lungs::interact(Erythrocyte& erythrocyte) {
	{
		lock_guard<mutex> lck {oxygenMtx};
		
		if (oxygen.size() > 0) {
			erythrocyte.takeOxygen(std::move(oxygen.back()));
			oxygen.pop_back();
		}
	}
	
	erythrocyte.setVein(vOut);
}

void Lungs::interact(Leukocyte& leukocyte) {
	leukocyte.setVein(vOut);
}

void Lungs::refresh() {
	box(win, 0, 0);
	mvwprintw(win, 0, 0, "Lungs");
	drawOxygen();
	wrefresh(win);
}

Coords Lungs::vOutPos() {
	return Coords{pos.line + 3, pos.col - 2};
}

void Lungs::setVeins(Vein* vIn, Vein* vOut) {
	this->vIn = vIn;
	this->vOut = vOut;
}