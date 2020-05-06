#pragma once
#include <mutex>

class Erythrocyte;

class Destination {
public:
	mutex accessMtx;
	virtual void interact(Erythrocyte& erythrocyte) = 0;
};