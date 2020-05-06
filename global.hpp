#pragma once

#include <ncurses.h>
#include <stdio.h>
#include <mutex>
#include <random>
#include <thread>
#include <memory>
#include <chrono>
#include <vector>
#include <forward_list>
#include "Coords.hpp"
using namespace std;

const int ESC = 27;

const int TERM_LINES = 24;
const int TERM_COLS = 80;

enum Color	{DEFAULT,
			INHALE,
			EXHALE,
			OXYGEN,
			VEIN,
			ERYTHROCYTE_O,
			ERYTHROCYTE_NO};

const int TASK_TIME_LB = 2500;
const int TASK_TIME_UB = 3500;

mutex endThreadsMtx;
bool endThreads = false;

mutex printMtx;		//do synchronizacji pisania na ekran

default_random_engine generator;

int randomTime(int a, int b) {		//zwraca liczbe calkowita losowa z przedzialu [a,b]
	uniform_int_distribution<int> distribution(a, b);
	return distribution(generator);
}

void initColors() {					//inicjalizuje pary kolorow
	init_pair(Color::DEFAULT, COLOR_WHITE, COLOR_BLACK);
	init_pair(Color::INHALE, COLOR_BLUE, COLOR_BLACK);
	init_pair(Color::EXHALE, COLOR_RED, COLOR_BLACK);
	init_pair(Color::OXYGEN, COLOR_WHITE, COLOR_CYAN);
	init_pair(Color::VEIN, COLOR_YELLOW, COLOR_BLACK);
	init_pair(Color::ERYTHROCYTE_O, COLOR_WHITE, COLOR_RED);
	init_pair(Color::ERYTHROCYTE_NO, COLOR_WHITE, COLOR_BLUE);
}

/* wClearLine, mvwprintw, mvwaddch synchronizowane przez blokowanie mutex printMtx */
void synch_wClearLine(WINDOW* win, int line, int start, int stop) {
	lock_guard<mutex> lck {printMtx};
	for (int i = start; i < stop; ++i) {
		mvwaddch(win, line, i, ' ');
	}
}

void synch_mvwprintw(WINDOW* win, int line, int col, int color, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char buffer[30];
	vsnprintf(buffer, 30, fmt, args);

	lock_guard<mutex> lck {printMtx};
	wattron(win, COLOR_PAIR(color));
	mvwprintw(win, line, col, buffer);
	wattroff(win, COLOR_PAIR(color));
}

void synch_mvwprintw(WINDOW* win, int line, int col, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char buffer[30];
	vsnprintf(buffer, 30, fmt, args);

	lock_guard<mutex> lck {printMtx};
	mvwprintw(win, line, col, buffer);
}

void synch_mvwaddch(WINDOW* win, int line, int col, char ch) {
	lock_guard<mutex> lck {printMtx};
	mvwaddch(win, line, col, ch);
}

void synch_mvwaddch(WINDOW* win, int line, int col, int color, char ch) {
	lock_guard<mutex> lck {printMtx};
	wattron(win, COLOR_PAIR(color));
	mvwaddch(win, line, col, ch);
	wattroff(win, COLOR_PAIR(color));
}