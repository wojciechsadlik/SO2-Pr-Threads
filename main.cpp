#include "global.hpp"
#include "Erythrocyte.hpp"
#include "Vein.hpp"
#include "Lungs.hpp"
#include "Heart.hpp"
#include "Fork.hpp"
#include "Junction.hpp"
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

	Lungs lungs{Coords{1, TERM_COLS / 3,}};
	Heart heart{Coords{9, TERM_COLS / 3}};
	Cell cell{Coords{17, TERM_COLS / 3}};
	Cell cell2{Coords{25, TERM_COLS / 3}};

	Vein vLH{lungs.vOutPos(), "lddddddr"};
	vLH.setDestination(&heart);

	Vein vHL{heart.outUpVPos(), "ruuuuuul"};
	vHL.setDestination(&lungs);

	Vein vHF{heart.outDownVPos(), "rdddddd"};
	Fork fork{&vHF};
	vHF.setDestination(&fork);

	Vein vFC{vHF.getEndPos(), "l"};
	vFC.setDestination(&cell);
	fork.addVein(&vFC);

	Vein vCJ{cell.vOutPos(), "l"};

	Vein vJH{vCJ.getEndPos(), "uuuuuur"};
	Junction junction{&vJH};
	vJH.setDestination(&heart);
	vCJ.setDestination(&junction);
	junction.addVein(&vJH);

	Vein vFC2{vHF.getEndPos(), "ddddddddl"};
	vFC2.setDestination(&cell2);
	fork.addVein(&vFC2);

	Vein vC2J{cell2.vOutPos(), "luuuuuuu"};
	vC2J.setDestination(&junction);
	junction.addVein(&vC2J);

	lungs.setVeins(&vHL, &vLH);
	heart.setVeins(&vLH, &vJH, &vHL, &vHF);
	cell.setVeins(&vFC, &vCJ);
	cell2.setVeins(&vFC2, &vC2J);

	mutex erListMtx;
	forward_list<Erythrocyte> erythrocytes;
	for (int i = 15; i > 0; --i)
		erythrocytes.emplace_front(i);

	thread lungsThd(ref(lungs));
	thread cellThd(ref(cell));
	thread cell2Thd(ref(cell2));

	forward_list<thread> erThds;
	for (auto& er : erythrocytes)
		erThds.emplace_front(ref(er));

	thread heartThd(ref(heart), &erythrocytes, &erListMtx);

	while (getch() != ESC) {
		vLH.draw();
		vHL.draw();
		vHF.draw();
		vFC.draw();
		vCJ.draw();
		vJH.draw();
		vFC2.draw();
		vC2J.draw();

		{
			lock_guard<mutex> lckErL{erListMtx};
			for (auto& er : erythrocytes)
				er.draw();
		}
			
		refresh();
		lungs.refresh();
		heart.refresh();
		cell.refresh();
		cell2.refresh();
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

	cell2Thd.join();
	cell2.refresh();

	for (auto& erThd : erThds) {
		erThd.join();
		refresh();
	}

	this_thread::sleep_for(chrono::seconds{2});

	endwin();

	return 0;
}
