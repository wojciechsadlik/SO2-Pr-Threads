#pragma once

#include "global.hpp"
#include "Destination.hpp"
#include "Vein.hpp"

class Junction: public Destination {
	Vein* vOut {nullptr};
	mutex accessMtx;

public:
	Junction(Vein* vOut);
	~Junction() = default;
	void interact(Erythrocyte& erythrocyte);
};

Junction::Junction(Vein* vOut): vOut(vOut) {
}

void Junction::interact(Erythrocyte& erythrocyte) {
	lock_guard<mutex> lcka {accessMtx};
	if (vOut != nullptr)
		erythrocyte.setVein(vOut);
}