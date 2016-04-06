#ifndef __TacticalAI_H__
#define __TacticalAI_H__

#include "j1Module.h"
#include <list>
#include <map>
#include <queue>

enum UNIT_EVENT{
END_MOVING,
ATTACKED,
ENEMY_TARGET,
GO_TO_POINT,
ENEMY_KILLED
};

class Unit;

class TacticalAI : public j1Module
{
public:

	TacticalAI();

	// Destructor
	virtual ~TacticalAI();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before quitting
	bool CleanUp();

	void SetEvent(UNIT_EVENT unit_event, Unit* unit, Unit* target = NULL);
	
private:
	map<Unit*, queue<UNIT_EVENT>> unit_queue;
};


#endif // __TacticalAI_H__