#pragma once

#include "global.hpp"
#include "Destination.hpp"
#include "Vein.hpp"

class Fork: public Destination {
	vector<Vein*> vOuts;
	int state {0};

public:
	Fork() = default;
	~Fork() = default;
	void addVein(Vein* vein);
	void interact(Erythrocyte& erythrocyte);
};

void Fork::addVein(Vein* vein) {
	vOuts.push_back(vein);
}

void Fork::interact(Erythrocyte& erythrocyte) {
	erythrocyte.setVein(vOuts[state]);

	if (vOuts.size() != (size_t) 0)
		state = (state + 1) % vOuts.size();
}