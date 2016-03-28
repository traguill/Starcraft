#ifndef __UNIT_H__
#define __UNIT_H__


#include "p2Defs.h"
#include "p2Point.h"
#include "Entity.h"
#include <vector>


enum UNIT_TYPE{ 
	MARINE, 
	FIREBAT, 
	GHOST, 
	MEDIC, 
	OBSERVER, 
	ENGINEER, 
	SHIP, 
	GOLIATH, 
	TANK, 
	VALKYRIE };

enum UNIT_STATE{
	UNIT_IDLE, 
	UNIT_MOVE,
	UNIT_ATTACK
};

class Unit : public Entity
{
	friend class j1EntityManager;	//Provisional

public:

	void Update(float dt);
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
	bool selected = false;
	UNIT_TYPE type;
	UNIT_STATE state;

};
#endif