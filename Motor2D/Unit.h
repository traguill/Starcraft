
#ifndef __UNIT_H__
#define __UNIT_H__

#include "p2Defs.h"
#include "p2Point.h"
#include "Entity.h"
#include <vector>


enum TYPE{ MARINE, FIREBAT, GHOST, MEDIC, OBSERVER, ENGINEER, SHIP, GOLIATH, TANK, VALKYRIE };
enum STATE{IDLE, MOVE, ATTACK};

class Unit : public Entity
{

public:

	bool Update(float dt);
	void Draw();

private:

	uint speed;
	uint damage;
	uint vision;
	uint range;
	uint cool;
	Entity* target;
	std::vector<iPoint> path;
	bool costume;
	TYPE type;
	STATE state;

};
#endif