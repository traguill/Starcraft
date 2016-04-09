#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Textures.h"
#include "j1FileSystem.h"
#include "TacticalAI.h"
#include "EntityManager.h"
#include "j1Pathfinding.h"
#include "j1Map.h"

TacticalAI::TacticalAI() : j1Module()
{
	name.append("tactical_ai");
}

// Destructor
TacticalAI::~TacticalAI()
{}

// Called before render is available
bool TacticalAI::Awake(pugi::xml_node& conf)
{
	return true;
	}

// Called before quitting
bool TacticalAI::CleanUp()
{
	
	return true;
}

bool TacticalAI::Update(float dt)
{


	//Check every x seconds if one unit is close to another
	list<Unit*>::iterator unit_f = App->entity->friendly_units.begin();

	while (unit_f != App->entity->friendly_units.end())
	{
		list<Unit*>::iterator unit_e = App->entity->enemy_units.begin();
		while (unit_e != App->entity->enemy_units.end())
		{
			if ((*unit_f)->state != UNIT_DIE && (*unit_e)->state != UNIT_DIE)
			{
				if ((*unit_f)->GetPosition().DistanceTo((*unit_e)->GetPosition()) <= (*unit_f)->vision)
				{
					if ((*unit_f)->GetTarget() == NULL)
					{
						LOG("Friend: I've found someone near");
						SetEvent(ENEMY_TARGET, *unit_f, *unit_e);
					}
					
					if ((*unit_e)->GetTarget() == NULL)
					{
						LOG("Enemy: I've found someone near");
						SetEvent(ENEMY_TARGET, *unit_e, *unit_f);
					}
				}
			}
			++unit_e;
		}
		++unit_f;
	}
	

	return true;
}


void TacticalAI::SetEvent(UNIT_EVENT unit_event, Unit* unit, Unit* target){

	switch (unit_event)
	{

	case ATTACKED:
		if (target)
		{
			if (unit->is_enemy)
				LOG("(Enemy):  I've been attack");
			else
				LOG("(Friend): I've been attack");
			SetEvent(ENEMY_TARGET, unit, target);
		}
		break;
	case ENEMY_TARGET:
		if (unit->GetPosition().DistanceTo(target->GetPosition()) <= unit->GetRange())
		{
			if (unit->is_enemy)
				LOG("(Enemy):  I'm going to attack");
			else
				LOG("(Friend): I'm going to attack");
			
			unit->SetTarget(target);
			unit->state = UNIT_ATTACK;
		}
		else
		{
			if (unit->is_enemy)
				LOG("Enemy: I have a target to kill, I'm going to move closer");
			else
				LOG("Friend: I have a target to kill, I'm going to move closer");

			CalculatePath(unit, target);
			unit->SetTarget(target);
			unit->events.push(ENEMY_TARGET);
		}
		break;
	case END_MOVING:
		if (unit->events.empty() == true)
		{
			unit->state = UNIT_IDLE;
		}
		else
		{
			UNIT_EVENT event = unit->events.front();
			unit->events.pop();
			if (unit->GetTarget() == NULL)
				unit->state = UNIT_IDLE;
			else
				SetEvent(event, unit, unit->GetTarget());
		}

		break;
	case GO_TO_POINT:
		//Pathfinding
		unit->state = UNIT_MOVE;
		break;
	case ENEMY_KILLED:
		bool enemy_found;

		if (unit->is_enemy == true)
		{
			enemy_found = SearchNearEnemyUnit(unit, App->entity->friendly_units);
		}
		else
		{
			enemy_found = SearchNearEnemyUnit(unit, App->entity->enemy_units);
		}
		
		if (enemy_found == false)
		{
			if (unit->is_enemy)
				LOG("(Enemy):  I've killed one enemy and no one is near");
			else
				LOG("(Friend): I've killed one enemy and no one is near");
			
			unit->state = UNIT_IDLE;

		}
		break;

	}


}

bool TacticalAI::CalculatePath(Unit* unit, Unit* target)
{
	bool ret = true;

	iPoint unit_map = App->map->WorldToMap(unit->GetPosition().x, unit->GetPosition().y, COLLIDER_MAP);

	iPoint target_map = App->map->WorldToMap(target->GetPosition().x, target->GetPosition().y, COLLIDER_MAP);

	vector<iPoint> path;

	if (App->pathfinding->CreateLine(unit_map, target_map) == true)
	{
		path.push_back(unit_map);
		path.push_back(target_map);

	}
	else
	{
		//Create pathfinding
		if (App->pathfinding->CreatePath(unit_map, target_map) == -1)
		{
			LOG("Impossible to create path");
			return false;
		}

		path = *App->pathfinding->GetLastPath();
	}

	unit->SetPath(path);

	return true; 
}

bool TacticalAI::SearchNearEnemyUnit(Unit* unit, list<Unit*> search_list)
{
	bool ret = false;
	list<Unit*>::iterator i = search_list.begin();

	while (i != search_list.end())
	{
		if ((*i)->GetPosition().DistanceTo(unit->GetPosition()) < unit->GetRange()) //Change range by vision range and maybe start pathfinding
		{
			if ((*i)->state != UNIT_DIE)
			{
				if (unit->is_enemy)
					LOG("Enemy: I've killed one enemy and another is near");
				else
					LOG("Friend: I've killed one enemy and another is near");
				SetEvent(ENEMY_TARGET, unit, (*i));
				return true;
			}	
		}
		++i;
	}

	return ret;
}