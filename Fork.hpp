#pragma once

#include "global.hpp"
#include "Destination.hpp"
#include "Vein.hpp"
#include "Erythrocyte.hpp"
#include "Leukocyte.hpp"

class Fork: public Destination {
	//vector<Vein*> vOuts;
	//int state {0};
	vector<int> keys;
	map<int, Vein*> vOuts;
	queue<int> leukocyteOrders;
	queue<int> erythrocyteOrders;
	
public:
	Fork() = default;
	~Fork() = default;
	void addVein(int id, Vein* vein);
	int getRandomKey();
	void interact(Erythrocyte& erythrocyte);
	void interact(Leukocyte& leukocyte);
};

void Fork::addVein(int id, Vein* vein) {
	vOuts.insert(pair<int, Vein*> {id, vein});
	keys.push_back(id);
}

int Fork::getRandomKey() {
	return keys[randomInt(0, keys.size() - 1)];
}

void Fork::interact(Erythrocyte& erythrocyte) {
	// erythrocyte.setVein(vOuts[state]);

	// if (vOuts.size() != (size_t) 0)
	// 	state = (state + 1) % vOuts.size();
	if (erythrocyteOrders.empty()) {
		erythrocyte.setVein(vOuts[getRandomKey()]);
	}
}

void Fork::interact(Leukocyte& leukocyte) {
	// leukocyte.setVein(vOuts[state]);

	// if (vOuts.size() != (size_t) 0)
	// 	state = (state + 1) % vOuts.size();
	if (leukocyteOrders.empty()) {
		leukocyte.setVein(vOuts[getRandomKey()]);
	}
}