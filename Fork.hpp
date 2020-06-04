#pragma once

#include "global.hpp"
#include "Destination.hpp"
#include "Vein.hpp"
#include "Erythrocyte.hpp"
#include "Leukocyte.hpp"

class Fork: public Destination {
	vector<int> keys;
	map<int, Vein*> vOuts;
	queue<int> leukocyteOrders;
	queue<int> erythrocyteOrders;
	mutex accessMtx;
	
public:
	Fork() = default;
	~Fork() = default;
	void addVein(int id, Vein* vein);
	int getRandomKey();
	void orderErythrocyte(int id);
	void orderLeukocyte(int id);
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

void Fork::orderErythrocyte(int id) {
	lock_guard<mutex> lck{accessMtx};
	erythrocyteOrders.push(id);
}

void Fork::orderLeukocyte(int id) {
	lock_guard<mutex> lck{accessMtx};
	leukocyteOrders.push(id);
}

void Fork::interact(Erythrocyte& erythrocyte) {
	lock_guard<mutex> lck{accessMtx};
	if (!erythrocyteOrders.empty()) {
		erythrocyte.setVein(vOuts[erythrocyteOrders.front()]);
		erythrocyteOrders.pop();
	} else {
		erythrocyte.setVein(vOuts[getRandomKey()]);
	}
}

void Fork::interact(Leukocyte& leukocyte) {
	lock_guard<mutex> lck{accessMtx};
	if (!leukocyteOrders.empty()) {
		leukocyte.setVein(vOuts[leukocyteOrders.front()]);
		leukocyteOrders.pop();
	} else {
		leukocyte.setVein(vOuts[getRandomKey()]);
	}
}