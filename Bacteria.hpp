#pragma once

#include "global.hpp"
#include "Cell.hpp"

class Bacteria {
	int id {0};
	Cell* cell;
	int line;
	const int COLS {21};
public:
	Bacteria(int id, Cell* cell);
	~Bacteria() = default;
	void sleep();
	void attack();
	void operator()();
};

Bacteria::Bacteria(int id, Cell* cell): id(id), cell(cell) {
	this->line = 2 * id;
}

void Bacteria::sleep() {
	synch_wClearLine(stdscr, line, 1, COLS);
	synch_mvwprintw(stdscr, line, 1, Color::BACTERIA_SLEEP, "bacteria%d: sleep", id);
	chrono::milliseconds taskTime {randomTime(TASK_TIME_LB, TASK_TIME_UB)};		//czas snu - [2.5s, 3.5s]
	for (int i = 1; i < COLS; ++i) {					//pasek postepu
		synch_mvwaddch(stdscr, line + 1, i, '*');
		this_thread::sleep_for(taskTime / COLS);
	}
	synch_wClearLine(stdscr, line, 1, COLS);
	synch_wClearLine(stdscr, line + 1, 1, COLS);
}

void Bacteria::attack() {
	synch_wClearLine(stdscr, line, 1, COLS);
	synch_mvwprintw(stdscr, line, 1, Color::BACTERIA_ATTACK, "bacteria%d: attacking", id);
	unique_lock<mutex> lck {cell->illnessMtx};
	cell->illness = true;
	cell->illnesscv.wait(lck);
}

void Bacteria::operator()() {
	while (true) {
		sleep();

		{
			lock_guard<mutex> lck {endThreadsMtx};
			if (endThreads) break;
		}

		if (random01() < 0.5)
			attack();
	}
	synch_wClearLine(stdscr, line, 1, COLS);
	synch_mvwprintw(stdscr, line, 1, Color::DEFAULT, "ended");
}