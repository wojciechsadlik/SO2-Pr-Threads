#include <ncurses.h>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <chrono>
#include <random>
#include <vector>
using namespace std;

const int ESC = 27;

const int TERM_LINES = 24;
const int TERM_COLS = 80;

enum Color	{DEFAULT,
			INHALE,
			EXHALE,
			OXYGEN,
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
	init_pair(Color::ERYTHROCYTE_O, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(Color::ERYTHROCYTE_NO, COLOR_WHITE, COLOR_RED);
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
/* synchronizowane wClearLine, mvwprintw, mvwaddch koniec */

/* mapa */
class SystemMap {

};
/* koniec mapa */

class Erythrocyte;
/* cel */
class Destination {
public:
	virtual void interact(Erythrocyte& erythrocyte) = 0;
};
/* koniec cel */

/* tlen */
class Oxygen {
	mutex mtx;
};
/* koniec tlen */

/* zyla */
class Vein {
	int startX, startY;
	vector<char> directions;
	Destination* destination;

public:
	mutex accessMtx;
	Vein(int startX, int startY, vector<char>& directions);
	~Vein() = default;
	vector<char>::iterator getIterator();
	char getDirection(int i);
	int getStartX();
	int getStartY();
	Destination* getDestination();
};

Vein::Vein(int startX, int startY, vector<char>& directions): 
startX(startX), startY(startY), directions(directions) {
}

vector<char>::iterator Vein::getIterator() {
	return directions.begin();
}

char Vein::getDirection(int i) {
	if ((size_t) i < directions.size()) return directions[i];
	else return 'x';
}

int Vein::getStartX() {
	return startX;
}

int Vein::getStartY() {
	return startY;
}

Destination* Vein::getDestination() {
	return destination;
}
/* koniec zyla */

/* rozwidlenie zyl*/
class Fork {
	Vein* vIn {nullptr};
	vector<Vein*> vOuts;
};
/* koniec rozwidlenie zyl */

/* serce */
class Heart : public Destination {
	Vein* leftVIn {nullptr};
	Vein* rightVIn {nullptr};
	Vein* leftVOut {nullptr};
	Vein* rightVOut {nullptr};

public:
	Heart();
	~Heart();
	void interact(Erythrocyte& erythrocyte);
};
/* koniec serce */

/* erytrocyt */
class Erythrocyte {
	int id;
	int x, y;
	Vein* vein {nullptr};
	unique_ptr<Oxygen> oxygen {nullptr};
	Destination* destination {nullptr};
	int nextDirection {0};

public:
	Erythrocyte();		// TODO: implement
	~Erythrocyte();		// TODO: implement
	void takeOxygen(unique_ptr<Oxygen> oxygen);	// TODO: implement
	void giveOxygen();	// TODO: implement
	void move();
	void operator()();
	void draw();
	void setVein(Vein* vein);
	void setXY(int x, int y);
	void setDestination(Destination* destination);
};

Erythrocyte::Erythrocyte() {
}

Erythrocyte::~Erythrocyte() {
	
}

void Erythrocyte::takeOxygen(unique_ptr<Oxygen> oxygen) {
	this->oxygen = std::move(oxygen);
}

void Erythrocyte::move() {
	if (vein != nullptr) {
		char c = '?';
		{
			lock_guard<mutex> lck {vein->accessMtx};
			c = vein->getDirection(nextDirection);
		}

		switch (c) {
			case 'u': ++y; break;
			case 'd': --y; break;
			case 'r': ++x; break;
			case 'l': --x; break;
			case 'x': destination->interact(*this); nextDirection = -1;
		}
		++nextDirection;
	}
}

void Erythrocyte::operator()() {
	while (true) {
		move();
		draw();
		this_thread::sleep_for(chrono::milliseconds(TASK_TIME_LB));
		{
			lock_guard<mutex> lck {endThreadsMtx};
			if (endThreads) break;
		}	
	}
	synch_mvwprintw(stdscr, y, x, Color::DEFAULT, "%02d", id);
}

void Erythrocyte::draw() {
	if (oxygen == nullptr)
		synch_mvwprintw(stdscr, y, x, Color::ERYTHROCYTE_NO, "%02d", id);
	else
		synch_mvwprintw(stdscr, y, x, Color::ERYTHROCYTE_O, "%02d", id);
}

void Erythrocyte::setVein(Vein* vein) {
	this->vein = vein;

	lock_guard<mutex> lck {vein->accessMtx};
	setXY(vein->getStartX(), vein->getStartY());
	setDestination(vein->getDestination());
}

void Erythrocyte::setXY(int x, int y) {
	this->x = x;
	this->y = y;
}

void Erythrocyte::setDestination(Destination* destination) {
	this->destination = destination;
}
/* koniec erytrocyt */

/* pluca */
class Lungs : public Destination{
	const int WIN_LINES {6};
	const int WIN_COLS {17};
	WINDOW* win {nullptr};
	vector<unique_ptr<Oxygen>> oxygen;
	mutex oxygenMtx;
	size_t capacity {17};	// maksymalna liczba jednostek tlenu
	Vein* vIn {nullptr};
	Vein* vOut {nullptr};

public:
	Lungs();
	~Lungs();
	void inhale();
	void exhale();
	void operator()();
	void drawOxygen();
	void refresh();
	void interact(Erythrocyte& erythrocyte);
};

Lungs::Lungs() {
	int col = TERM_COLS / 2 - WIN_COLS / 2;
	int line = 2;
	win = newwin(WIN_LINES, WIN_COLS, line, col);
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
		{
			lock_guard<mutex> lck {endThreadsMtx};
			if (endThreads) break;
		}	
	}
	synch_wClearLine(win, 1, 1, WIN_COLS - 1);
	synch_wClearLine(win, 2, 1, WIN_COLS - 1);
	synch_mvwprintw(win, 1, 1, Color::DEFAULT, "ended");
	this->refresh();
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
			if (oxygen.size() > 0)
				oxygen.pop_back();
		}
		synch_mvwaddch(win, 2, i, '*');
		this_thread::sleep_for(taskTime / WIN_COLS);
	}
}

void Lungs::drawOxygen() {
	synch_wClearLine(win, 3, 1, WIN_COLS - 1);
	synch_wClearLine(win, 4, 1, WIN_COLS - 1);

	for (size_t i = 0; i < oxygen.size(); i++) {
		synch_mvwaddch(win, 3 + i / (WIN_COLS - 1), 1 + i % (WIN_COLS - 1), Color::OXYGEN, ' ');
	}
}

void Lungs::interact(Erythrocyte& erythrocyte) {
	if (oxygen.size() > 0) {
		lock_guard<mutex> lck {oxygenMtx};
		erythrocyte.takeOxygen(std::move(oxygen.back()));
		oxygen.pop_back();
	}
}

void Lungs::refresh() {
	drawOxygen();
	wrefresh(win);
}
/* koniec pluca */

/* komorka */
class Cell: public Destination {
	WINDOW* win = nullptr;
	Vein* vIn {nullptr};
	Vein* vOut {nullptr};

	Cell();					// TODO: implement
	~Cell();				// TODO: implement
	void waitForOxygen();	// TODO: implement
	void processOxygen();	// TODO: implement
	void operator()();		// TODO: implement
};
/* koniec komorka*/
 
int main(int argc, char* argv[])
{
	initscr();						//inicjalizacja terminala curses
	start_color();					//uruchomienie obslugi kolorow
	noecho();						//nie wyswietlaj wciskanych klawiszy
	curs_set(0);					//nie wyswietlaj kursora
	keypad(stdscr, true);			//obsluga klawiszy specjalnych
	timeout(25);					//timeout dla getch()

	initColors();

	refresh();

	Lungs lungs;
	Erythrocyte er;
	lungs.interact(er);
	thread lungsT(ref(lungs));
 
	while (getch() != ESC) {
		refresh();
		lungs.refresh();
		this_thread::sleep_for(chrono::milliseconds{50});
	}

	{
		lock_guard<mutex> lck {endThreadsMtx};
		endThreads = true;
	}

	lungsT.join();

	this_thread::sleep_for(chrono::seconds{1});

	endwin();

	return 0;
}
