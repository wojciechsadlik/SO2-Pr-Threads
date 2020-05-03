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
	init_pair(Color::VEIN, COLOR_BLACK, COLOR_WHITE);
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
/* synchronizowane wClearLine, mvwprintw, mvwaddch koniec */

struct Coords {
	int line = 0;
	int col = 0;
	Coords() = default;
	Coords(int line, int col): line(line), col(col){};
	~Coords() = default;
};

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
	Coords startPos;
	vector<char> directions;
	Destination* destination;

public:
	mutex entranceMtx;
	mutex accessMtx;
	Vein(Coords startPos, vector<char>& directions);
	~Vein() = default;
	vector<char>::iterator getIterator();
	char getDirection(int i);
	Coords getStartPos();
	void setDestination(Destination* destination);
	Destination* getDestination();
	void draw();
};

Vein::Vein(Coords startPos, vector<char>& directions): 
startPos(startPos), directions(directions) {
}

vector<char>::iterator Vein::getIterator() {
	return directions.begin();
}

char Vein::getDirection(int i) {
	if ((size_t) i < directions.size()) return directions[i];
	else return 'x';
}

Coords Vein::getStartPos() {
	return startPos;
}

void Vein::setDestination(Destination* destination) {
	this->destination = destination;
}

Destination* Vein::getDestination() {
	return destination;
}

void Vein::draw() {
	int col = startPos.col;
	int line = startPos.line;
	lock_guard<mutex> lck {accessMtx};
	lock_guard<mutex> lckp {printMtx};
	wattron(stdscr, COLOR_PAIR(Color::VEIN));
	for (int i = 0; (size_t) i < directions.size(); ++i) {
		char c = directions[i];
		mvprintw(line, col, " %c", c);
		switch (c) {
			case 'u': --line; break;
			case 'd': ++line; break;
			case 'r': col += 2; break;
			case 'l': col -= 2; break;
		}
	}
	mvprintw(line, col, " x");
	wattroff(stdscr, COLOR_PAIR(Color::VEIN));
}
/* koniec zyla */

/* rozwidlenie zyl*/
class Fork {
	Vein* vIn {nullptr};
	vector<Vein*> vOuts;
};
/* koniec rozwidlenie zyl */

/* erytrocyt */
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
	Erythrocyte();		// TODO: implement
	~Erythrocyte();		// TODO: implement
	void takeOxygen(unique_ptr<Oxygen> oxygen);
	void giveOxygen();
	bool move();
	void operator()();
	void draw();
	void setVein(Vein* vein);
	void setPos(Coords pos);
	void setDestination(Destination* destination);
};

Erythrocyte::Erythrocyte() {
}

Erythrocyte::~Erythrocyte() {
}

void Erythrocyte::takeOxygen(unique_ptr<Oxygen> oxygen) {
	lock_guard<mutex> lcks {setable};
	this->oxygen = std::move(oxygen);
}

void Erythrocyte::giveOxygen() {
	lock_guard<mutex> lcks {setable};
	oxygen.release();
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
			if (entranceLck) {
				entranceLck.unlock();
				entranceLck.release();
			}
		}

		if (t) destination->interact(*this);

		this_thread::sleep_for(chrono::milliseconds(1000));
		
		{
			lock_guard<mutex> lck {endThreadsMtx};
			if (endThreads) break;
		}	
	}
	synch_mvwprintw(stdscr, pos.line, pos.col, Color::DEFAULT, "%02d", id);
}

void Erythrocyte::draw() {
	lock_guard<mutex> lcks {setable};
	if (oxygen == nullptr)
		synch_mvwprintw(stdscr, pos.line, pos.col, Color::ERYTHROCYTE_NO, "%02d", id);
	else
		synch_mvwprintw(stdscr, pos.line, pos.col, Color::ERYTHROCYTE_O, "%02d", id);
}

void Erythrocyte::setVein(Vein* vein) {
	lock_guard<mutex> lcks {setable};
	
	this->vein = vein;

	entranceLck = unique_lock<mutex>{vein->entranceMtx};
	lock_guard<mutex> lck {vein->accessMtx};
	setPos(vein->getStartPos());
	setDestination(vein->getDestination());
}

void Erythrocyte::setPos(Coords pos) {
	this->pos = pos;
}

void Erythrocyte::setDestination(Destination* destination) {
	this->destination = destination;
}
/* koniec erytrocyt */

/* serce */
class Heart : public Destination {
	Coords pos;
	const int WIN_LINES {6};
	const int WIN_COLS {17};
	WINDOW* win {nullptr};
	Vein* leftUpV {nullptr};
	Vein* leftDownV {nullptr};
	Vein* rightUpV {nullptr};
	Vein* rightDownV {nullptr};

public:
	mutex accessMtx;
	Heart(Coords pos);
	~Heart();
	void setVeins(Vein* leftUpV, Vein* leftDownV, Vein* rightUpV, Vein* rightDownV);
	void refresh();
	void addErytrocyte(Erythrocyte& erythrocyte);
	void interact(Erythrocyte& erythrocyte);
	Coords rightUpVPos();
	Coords rightDownVPos();
};

Heart::Heart(Coords pos): pos(pos) {
	win = newwin(WIN_LINES, WIN_COLS, pos.line, pos.col);
	box(win, 0, 0);
	mvwprintw(win, 0, 0, "Heart");
}

Heart::~Heart() {
	delwin(win);
}

void Heart::setVeins(Vein* leftUpV, Vein* leftDownV, Vein* rightUpV, Vein* rightDownV){
	this->leftUpV = leftUpV;
	this->leftDownV = leftDownV;
	this->rightUpV = rightUpV;
	this->rightDownV = rightDownV;
}

void Heart::refresh() {
	box(win, 0, 0);
	mvwprintw(win, 0, 0, "Heart");
	wrefresh(win);
}

void Heart::addErytrocyte(Erythrocyte& erythrocyte) {
	erythrocyte.setVein(rightUpV);
}

void Heart::interact(Erythrocyte& erythrocyte) {
	erythrocyte.giveOxygen();
	erythrocyte.setVein(rightUpV);
}

Coords Heart::rightUpVPos() {
	return Coords{pos.line + 1, pos.col + WIN_COLS};
}
/* koniec serce */

/* pluca */
class Lungs : public Destination{
	Coords pos;
	const int WIN_LINES {6};
	const int WIN_COLS {17};
	WINDOW* win {nullptr};
	vector<unique_ptr<Oxygen>> oxygen;
	mutex oxygenMtx;
	size_t capacity {17};	// maksymalna liczba jednostek tlenu
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

	for (size_t i = 0; i < oxygen.size(); i++) {
		synch_mvwaddch(win, 3 + i / (WIN_COLS - 2), 1 + i % (WIN_COLS - 2), Color::OXYGEN, ' ');
	}
}

void Lungs::interact(Erythrocyte& erythrocyte) {
	if (oxygen.size() > 0) {
		lock_guard<mutex> lck {oxygenMtx};
		erythrocyte.takeOxygen(std::move(oxygen.back()));
		oxygen.pop_back();
	}
	erythrocyte.setVein(vOut);
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

	Lungs lungs{Coords{2, TERM_COLS / 3,}};
	Heart heart{Coords{10, TERM_COLS / 3}};

	vector<char> vpath {'l', 'd', 'd', 'd', 'd', 'd', 'd', 'r'};
	Vein vLH{lungs.vOutPos(), vpath};
	vLH.setDestination(&heart);

	vpath = vector<char>{'r', 'u', 'u', 'u', 'u', 'u', 'u', 'l'};
	Vein vHL{heart.rightUpVPos(), vpath};
	vHL.setDestination(&lungs);

	lungs.setVeins(&vHL, &vLH);
	heart.setVeins(&vLH, nullptr, &vHL, nullptr);

	Erythrocyte er, er2;

	thread lungsThd(ref(lungs));
	thread erThd(ref(er));
	thread er2Thd(ref(er2));

	heart.addErytrocyte(ref(er));
	heart.addErytrocyte(ref(er2));
 
	while (getch() != ESC) {
		vLH.draw();
		vHL.draw();
		er.draw();
		er2.draw();
		refresh();
		lungs.refresh();
		heart.refresh();
		this_thread::sleep_for(chrono::milliseconds{20});
	}

	{
		lock_guard<mutex> lck {endThreadsMtx};
		endThreads = true;
	}

	lungsThd.join();
	erThd.join();
	er2Thd.join();

	this_thread::sleep_for(chrono::seconds{1});

	endwin();

	return 0;
}
