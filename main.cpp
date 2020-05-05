#include "utils.hpp"
#include "Erythrocyte.hpp"
#include "Vein.hpp"
#include "Lungs.hpp"
#include "Heart.hpp"
#include "Cell.hpp"

using namespace std;
 
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
	Vein vLH{0, lungs.vOutPos(), vpath};
	vLH.setDestination(&heart);

	vpath = vector<char>{'r', 'u', 'u', 'u', 'u', 'u', 'u', 'l'};
	Vein vHL{1, heart.outUpVPos(), vpath};
	vHL.setDestination(&lungs);

	vpath = vector<char>{'r','d','d','d','l','l','l','l','l','l',
						'l','l','l','l','l','l', 'u','u','u','r'};
	Vein vHH{2, heart.outDownVPos(), vpath};
	vHH.setDestination(&heart);

	lungs.setVeins(&vHL, &vLH);
	heart.setVeins(&vLH, &vHH, &vHL, &vHH);

	forward_list<Erythrocyte> erythrocytes;
	for (int i = 0; i < 3; ++i)
		erythrocytes.emplace_front();

	thread lungsThd(ref(lungs));
	forward_list<thread> erThds;
	for (auto& er : erythrocytes) {
		erThds.emplace_front(ref(er));
		heart.addErytrocyte(ref(er));
	}
 
	while (getch() != ESC) {
		vLH.draw();
		vHL.draw();
		vHH.draw();
		for (auto& er : erythrocytes)
			er.draw();
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
	lungs.refresh();

	for (auto& erThd : erThds) {
		erThd.join();
		refresh();
	}

	this_thread::sleep_for(chrono::seconds{1});

	endwin();

	return 0;
}
