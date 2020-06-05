#pragma once

#include "global.hpp"
#include "Destination.hpp"
#include "Vein.hpp"
#include "Oxygen.hpp"
#include "Fork.hpp"

class Cell: public Destination {
	int id;
	Coords pos;
	const int WIN_LINES {6};
	const int WIN_COLS {17};
	WINDOW* win {nullptr};
	Vein* vIn {nullptr};
	Vein* vOut {nullptr};
	unique_ptr<Oxygen> oxygen {nullptr};
	mutex accessMtx;
	list<Fork*> forks;
	bool leukocyteOrdered {false};

public:
	mutex illnessMtx;
	condition_variable illnesscv;
	bool illness;
	Cell(int id, Coords pos);
	~Cell();
	int getId();
	void setForks(list<Fork*> forks);
	void orderErythrocyte();
	void orderLeukocyte();
	void waitForOxygen();
	void processOxygen();
	void operator()();
	void interact(Erythrocyte& erythrocyte);
	void interact(Leukocyte& leukocyte);
	void setVeins(Vein* vIn, Vein* vOut);
	Coords vOutPos();
	void refresh();
};

Cell::Cell(int id, Coords pos): id(id), pos(pos) {
	illness = false;
	win = newwin(WIN_LINES, WIN_COLS, pos.line, pos.col);
	box(win, 0, 0);
	mvwprintw(win, 0, 0, "Cell");
}

Cell::~Cell() {
	delwin(win);
}

int Cell::getId() {
	lock_guard<mutex> lckd {accessMtx};
	return id;
}

void Cell::setForks(list<Fork*> forks) {
	lock_guard<mutex> lckd {accessMtx};
	this->forks = forks;
}

void Cell::orderErythrocyte() {
	lock_guard<mutex> lckd {accessMtx};
	for (auto f : forks) {
		f->orderErythrocyte(id);
	}
}

void Cell::orderLeukocyte() {
	lock_guard<mutex> lckd {accessMtx};
	for (auto f : forks) {
		f->orderLeukocyte(id);
	}
	leukocyteOrdered = true;
}

void Cell::waitForOxygen() {
	synch_wClearLine(win, 1, 1, WIN_COLS - 1);
	synch_wClearLine(win, 2, 1, WIN_COLS - 1);
	synch_mvwprintw(win, 1, 1, Color::DEFAULT, "waiting");
	
	while (true) {
		{
			lock_guard<mutex> lckd {accessMtx};
			if (oxygen != nullptr) break;
		}
		{
			lock_guard<mutex> lck {illnessMtx};		//zeby sprawdzic czy bakterie zaatakowaly
			if (illness) {							//jezeli zaatakowaly to wypisz innym kolorem
				synch_mvwprintw(win, 1, 1, Color::CELL_ILL, "waiting");
				if (!leukocyteOrdered)				//jezeli nie zamowiono leukocytu
					orderLeukocyte();				//to zamow leukocyt
			}
			else
				synch_mvwprintw(win, 1, 1, Color::DEFAULT, "waiting");
		}

		this_thread::sleep_for(chrono::milliseconds{500});

		lock_guard<mutex> lcke {endThreadsMtx};
		if (endThreads) return;
	}
	oxygen.release();
}

void Cell::processOxygen() {
	synch_wClearLine(win, 1, 1, WIN_COLS - 1);
	synch_wClearLine(win, 2, 1, WIN_COLS - 1);
	synch_mvwprintw(win, 1, 1, Color::DEFAULT, "processing");
	chrono::milliseconds taskTime {randomTime(TASK_TIME_LB, TASK_TIME_UB)};		//czas snu - [2.5s, 3.5s]
	{
		lock_guard<mutex> lck {illnessMtx};
		if (illness) {
			taskTime /= 2;
			synch_mvwprintw(win, 1, 1, Color::CELL_ILL, "processing");
		}
	}
	for (int i = 1; i < WIN_COLS - 1; ++i) {					//pasek postepu
		{
			unique_lock<mutex> lckd {accessMtx};				//zeby mozna bylo odblokowac przed zamowieniem leukocytus
			lock_guard<mutex> lcki {illnessMtx};
			if (oxygen != nullptr) {							//jezeli w trakcie przetwarzania otrzymano nowy tlen
				synch_wClearLine(win, 2, 1, WIN_COLS - 1);
				i = 1;											//resetuj stan przetwarzania
				oxygen.release();
				taskTime = randomTime(TASK_TIME_LB, TASK_TIME_UB);
				if (illness)
					taskTime /= 2;
			}
			
			if (illness) {										//jezeli zaatakowaly bakterie
				synch_mvwprintw(win, 1, 1, Color::CELL_ILL, "processing");	//oznacz innym kolorem
				if (!leukocyteOrdered) {						//jezeli nie zamowiono leukocytu
					taskTime /= 2;								//skroc czas przetwarzania
					lckd.unlock();								//zeby mozna bylo zamowic
					orderLeukocyte();							//zamow leukocyt
				}
			} else {
				synch_mvwprintw(win, 1, 1, Color::DEFAULT, "processing");
			}
		}

		synch_mvwaddch(win, 2, i, '*');
		this_thread::sleep_for(taskTime / WIN_COLS);
		
		lock_guard<mutex> lcke {endThreadsMtx};
		if (endThreads) return;
	}
}

void Cell::operator()() {
	while (true) {
		orderErythrocyte();
		waitForOxygen();
		processOxygen();

		lock_guard<mutex> lck {endThreadsMtx};
		if (endThreads) break;
	}

	illnesscv.notify_one();		//jezeli koniec to powiadom czekajace bakterie, zeby mogly sie zakonczyc

	synch_wClearLine(win, 1, 1, WIN_COLS - 1);
	synch_wClearLine(win, 2, 1, WIN_COLS - 1);
	synch_mvwprintw(win, 1, 1, Color::DEFAULT, "ended");
}

void Cell::interact(Erythrocyte& erythrocyte) {
	lock_guard<mutex> lckd {accessMtx};
	oxygen = erythrocyte.giveOxygen();
	erythrocyte.setVein(vOut);
}

void Cell::interact(Leukocyte& leukocyte) {
	lock_guard<mutex> lckd {accessMtx};
	{
		lock_guard<mutex> lck {illnessMtx};		//zeby mozna bylo sprawdzic czy bakterie atakuja
		if (illness) {							//jezeli tak
			illness = false;					//to wylecz
			illnesscv.notify_one();				//i powiadom bakterie, ze maja skonczyc
			leukocyteOrdered = false;			//zresetuj stan zamowienia
		}
	}
	leukocyte.setVein(vOut);
}

void Cell::setVeins(Vein* vIn, Vein* vOut) {
	lock_guard<mutex> lckd {accessMtx};
	this->vIn = vIn;
	this->vOut = vOut;
}

Coords Cell::vOutPos() {
	lock_guard<mutex> lckd {accessMtx};
	return Coords{pos.line + 2, pos.col - 2};
}

void Cell::refresh() {
	box(win, 0, 0);
	mvwprintw(win, 0, 0, "Cell");
	wrefresh(win);
}