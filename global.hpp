#pragma once

#include <ncurses.h>
#include <stdio.h>
#include <mutex>
#include <condition_variable>
#include <random>
#include <thread>
#include <memory>
#include <chrono>
#include <vector>
#include <list>
#include <forward_list>
#include <map>
#include <queue>
#include "Coords.hpp"
using namespace std;

const int ESC = 27;

const int TERM_LINES = 24;
const int TERM_COLS = 80;

const int ER_COUNT = 14;
const int LEUK_COUNT = 6;

enum Color	{DEFAULT,
			INHALE,
			EXHALE,
			OXYGEN,
			VEIN,
			ERYTHROCYTE_O,
			ERYTHROCYTE_NO,
			LEUKOCYTE,
			BACTERIA,
			BACTERIA_SLEEP,
			BACTERIA_ATTACK,
			CELL_ILL};

const int TASK_TIME_LB = 2500;
const int TASK_TIME_UB = 3500;

mutex endThreadsMtx;		//do zakonczenia watkow
bool endThreads = false;	//do zakonczenia watkow

condition_variable beatcv;	//do synchronizowania ruchu krwinek w takcie bicia serca
mutex beatmtx;				//do synchronizowania ruchu krwinek w takcie bicia serca

mutex printMtx;		//do synchronizacji pisania na ekran

default_random_engine generator;

chrono::milliseconds randomTime(int a, int b) {		//zwraca losowy czas z przedzialu [a,b] w ms
	uniform_int_distribution<int> distribution(a, b);
	return chrono::milliseconds {distribution(generator)};
}

int randomInt(int a, int b) {					//zwraca liczbe losowa calkowita z przedzialu [a,b]
	uniform_int_distribution<int> distribution(a, b);
	return distribution(generator);
}

double random01() {
	uniform_real_distribution<double> distribution(0,1);
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
	init_pair(Color::LEUKOCYTE, COLOR_BLACK, COLOR_WHITE);
	init_pair(Color::BACTERIA, COLOR_GREEN, COLOR_BLACK);
	init_pair(Color::BACTERIA_SLEEP, COLOR_GREEN, COLOR_BLACK);
	init_pair(Color::BACTERIA_ATTACK, COLOR_GREEN, COLOR_BLACK);
	init_pair(Color::CELL_ILL, COLOR_BLACK, COLOR_GREEN);
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