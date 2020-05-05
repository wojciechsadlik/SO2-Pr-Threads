#pragma once

class Erythrocyte;

class Destination {
public:
	virtual void interact(Erythrocyte& erythrocyte) = 0;
};