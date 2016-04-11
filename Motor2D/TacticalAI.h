#ifndef __TacticalAI_H__
#define __TacticalAI_H__

#include "j1Module.h"
#include <list>
#include <map>
#include <queue>

#define COLLISION_DISTANCE 15 //Radius of 'vital' space that every unit have to avoid collisons

enum UNIT_EVENT{
END_MOVING,
ATTACKED,
ENEMY_TARGET,
GO_TO_POINT,
ENEMY_KILLED,
ENEMY_RUNNING
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

	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

	void SetEvent(UNIT_EVENT unit_event, Unit* unit, Unit* target = NULL);
	
private:

	//Calculates the path to the target
	bool CalculatePath(Unit* unit, Unit* target);

	//Search near enemies and target them (enemy->friendly friendly->enemy)
	bool SearchNearEnemyUnit(Unit* unit, list<Unit*> search_list);

	//Collisions
	void CheckCollisions(); //Only between units
	void CheckCollisionsLists(list<Unit*> list_a, list<Unit*> list_b);
	void SeparateUnits(Unit* unit_a, Unit* unit_b);

	void SeparateAtkUnits(Unit* unit, Unit* reference);

	void SeparateIdleUnits(Unit* unit_a, Unit* unit_b, bool both_idle = false); //Separates moving->idle  & idle->idle

	bool OverlapRectangles(const SDL_Rect r1,const SDL_Rect r2)const;

	//If one unit sees another kill him
	void Vision();

private:

	//Check vision and collisions 5 times / sec
	float checks = 0.12f;
	float actual_time = 0;
};


#endif // __TacticalAI_H__