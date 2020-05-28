#pragma once
#include <mutex>

class Erythrocyte;
class Leukocyte;

class Destination {
public:
	virtual void interact(Erythrocyte& erythrocyte) = 0;
	virtual void interact(Leukocyte& Leukocyte) = 0;
};