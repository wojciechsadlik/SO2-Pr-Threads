#include "global.hpp"
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
	Cell cell{Coords{18, TERM_COLS / 3}};

	vector<char> vpath {'l', 'd', 'd', 'd', 'd', 'd', 'd', 'r'};
	Vein vLH{0, lungs.vOutPos(), vpath};
	vLH.setDestination(&heart);

	vpath = vector<char>{'r', 'u', 'u', 'u', 'u', 'u', 'u', 'l'};
	Vein vHL{1, heart.outUpVPos(), vpath};
	vHL.setDestination(&lungs);

	vpath = vector<char>{'r', 'd', 'd', 'd', 'd', 'd', 'd', 'l'};
	Vein vHC{2, heart.outDownVPos(), vpath};
	vHC.setDestination(&cell);

	vpath = vector<char>{'l', 'u', 'u', 'u', 'u', 'u', 'u', 'r'};
	Vein vCH{3, cell.vOutPos(), vpath};
	vCH.setDestination(&heart);

	lungs.setVeins(&vHL, &vLH);
	heart.setVeins(&vLH, &vCH, &vHL, &vHC);
	cell.setVeins(&vHC, &vCH);

	mutex erListMtx;
	forward_list<Erythrocyte> erythrocytes;
	for (int i = 10; i > 0; --i)
		erythrocytes.emplace_front(i);

	thread lungsThd(ref(lungs));
	thread cellThd(ref(cell));

	forward_list<thread> erThds;
	for (auto& er : erythrocytes)
		erThds.emplace_front(ref(er));

	thread heartThd(ref(heart), &erythrocytes, &erListMtx);

	while (getch() != ESC) {
		vLH.draw();
		vHL.draw();
		vHC.draw();
		vCH.draw();
		{
			lock_guard<mutex> lckErL{erListMtx};
			for (auto& er : erythrocytes) {
				unique_lock<mutex> lckErA{er.accessMtx, try_to_lock};
				if (lckErA)	er.draw();
			}
		}
			
		refresh();
		lungs.refresh();
		heart.refresh();
		cell.refresh();
		this_thread::sleep_for(chrono::milliseconds{20});
	}

	{
		lock_guard<mutex> lck {endThreadsMtx};
		endThreads = true;
	}

	heartThd.join();
	heart.refresh();

	lungsThd.join();
	lungs.refresh();

	cellThd.join();
	cell.refresh();

	for (auto& erThd : erThds) {
		erThd.join();
		refresh();
	}

	this_thread::sleep_for(chrono::seconds{2});

	endwin();

	return 0;
}
