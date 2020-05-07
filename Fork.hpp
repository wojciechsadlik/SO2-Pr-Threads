#pragma once

#include "global.hpp"
#include "Destination.hpp"
#include "Vein.hpp"

class Fork: public Destination {
	Vein* vIn {nullptr};
	vector<Vein*> vOuts;
	int state {0};

public:
	Fork(Vein* vIn);
	~Fork() = default;
	void addVein(Vein* vein);
	void interact(Erythrocyte& erythrocyte);
};

Fork::Fork(Vein* vIn): vIn(vIn) {
	state = 0;
}

void Fork::addVein(Vein* vein) {
	vOuts.push_back(vein);
}

void Fork::interact(Erythrocyte& erythrocyte) {
	erythrocyte.setVein(vOuts[state]);

	if (vOuts.size() != (size_t) 0)
		state = (state + 1) % vOuts.size();
}