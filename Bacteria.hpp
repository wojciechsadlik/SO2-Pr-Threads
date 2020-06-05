#pragma once

#include "global.hpp"
#include "Cell.hpp"

class Bacteria {
	int id {0};
	Cell* cell;
	int line;
	const int COLS {21};
	WINDOW* win;
public:
	Bacteria(int id, Cell* cell);
	~Bacteria();
	void sleep();
	void attack();
	void operator()();
	void refresh();
};

Bacteria::Bacteria(int id, Cell* cell): id(id), cell(cell) {
	this->line = 2 * id;
	win = newwin(2, COLS, line, 0);
}

Bacteria::~Bacteria() {
	delwin(win);
}

void Bacteria::sleep() {
	synch_mvwprintw(win, 0, 1, Color::BACTERIA_SLEEP, "bacteria%d: sleep", id);
	chrono::milliseconds taskTime {randomTime(TASK_TIME_LB, 2 * TASK_TIME_UB)};		//czas snu - [2.5s, 2 * 3.5s]
	for (int i = 1; i < COLS; ++i) {					//pasek postepu
		synch_mvwaddch(win, 1, i, '*');
		this_thread::sleep_for(taskTime / COLS);
		{
			lock_guard<mutex> lck {endThreadsMtx};
			if (endThreads) break;
		}
	}
	synch_wClearLine(win, 0, 1, COLS);
	synch_wClearLine(win, 1, 1, COLS);
}

void Bacteria::attack() {
	synch_mvwprintw(win, 0, 1, Color::BACTERIA_ATTACK, "bacteria%d: attacking", id);
	unique_lock<mutex> lck {cell->illnessMtx};
	cell->illness = true;
	cell->illnesscv.wait(lck);			//czeka na sygnal o przyjsciu leukocytu
	synch_wClearLine(win, 0, 1, COLS);
}

void Bacteria::operator()() {
	while (true) {
		sleep();

		{
			lock_guard<mutex> lck {endThreadsMtx};
			if (endThreads) break;
		}

		attack();
	}
	synch_wClearLine(win, 0, 1, COLS);
	synch_mvwprintw(win, 0, 1, Color::DEFAULT, "ended");
}

void Bacteria::refresh() {
	wrefresh(win);
}