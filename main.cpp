#include "global.hpp"
#include "Erythrocyte.hpp"
#include "Leukocyte.hpp"
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

	Vein vHF{heart.outDownVPos(), "rddddd"};
	Fork fork;
	vHF.setDestination(&fork);

	Vein vFC{vHF.getEndPos(), "l"};
	vFC.setDestination(&cell);
	fork.addVein(&vFC);

	Vein vCJ{cell.vOutPos(), "l"};

	Vein vJH{vCJ.getEndPos(), "uuuuuurr"};
	Junction junction{&vJH};
	vJH.setDestination(&heart);
	vCJ.setDestination(&junction);

	Vein vFC2{vHF.getEndPos(), "ddddddddl"};
	vFC2.setDestination(&cell2);
	fork.addVein(&vFC2);

	Vein vC2J{cell2.vOutPos(), "lluuuuuuu"};
	vC2J.setDestination(&junction);

	lungs.setVeins(&vHL, &vLH);
	heart.setVeins(&vLH, &vJH, &vHL, &vHF);
	cell.setVeins(&vFC, &vCJ);
	cell2.setVeins(&vFC2, &vC2J);

	mutex erListMtx;
	forward_list<Erythrocyte> erythrocytes;
	for (int i = ER_COUNT; i > 0; --i)
		erythrocytes.emplace_front(i);

	mutex leukListMtx;
	forward_list<Leukocyte> leukocytes;
	for (int i = LEUK_COUNT; i > 0; --i)
		leukocytes.emplace_front(i);

	thread lungsThd(ref(lungs));
	thread cellThd(ref(cell));
	thread cell2Thd(ref(cell2));

	forward_list<thread> erThds;
	for (auto& er : erythrocytes)
		erThds.emplace_front(ref(er));

	forward_list<thread> leukThds;
	for (auto& leuk : leukocytes)
		leukThds.emplace_front(ref(leuk));

	thread heartThd(ref(heart), &erythrocytes, &erListMtx, &leukocytes, &leukListMtx);

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

		{
			lock_guard<mutex> lckLeukL{leukListMtx};
			for (auto& leuk : leukocytes)
				leuk.draw();
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

	for (auto& leukThd : leukThds) {
		leukThd.join();
		refresh();
	}

	this_thread::sleep_for(chrono::seconds{2});

	endwin();

	return 0;
}
