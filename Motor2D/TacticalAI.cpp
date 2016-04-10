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


	Vision();


	CheckCollisions();
	

	return true;
}


void TacticalAI::SetEvent(UNIT_EVENT unit_event, Unit* unit, Unit* target){

	//Do not change the state if the unit is resolving a collision (high priority)
	if (unit->resolving_collision == true)
	{
		LOG("Changing state while resolving collision");
		return;
	}

	if (unit->state == UNIT_DIE)
	{
		LOG("Trying to change state while im dead");
	}



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


void TacticalAI::CheckCollisions()
{
	CheckCollisionsLists(App->entity->friendly_units, App->entity->friendly_units);
	CheckCollisionsLists(App->entity->friendly_units, App->entity->enemy_units);
	CheckCollisionsLists(App->entity->enemy_units, App->entity->enemy_units);
}

void TacticalAI::CheckCollisionsLists(list<Unit*> list_a, list<Unit*> list_b)
{
	list<Unit*>::iterator unit_a = list_a.begin();
	int count_a = 1;

	while (unit_a != list_a.end())
	{
		list<Unit*>::iterator unit_b = list_b.begin();
		int count_b = 1;
		while (unit_b != list_b.end())
		{
			if (count_a >= count_b)	//Avoids duplicate searches
			{
				++count_b;
				++unit_b;
				continue;
			}
			//First check if someone is resolving collisions
			if ((*unit_a)->resolving_collision == false && (*unit_b)->resolving_collision == false) 
			{
				if (OverlapRectangles((*unit_a)->GetCollider(), (*unit_b)->GetCollider()))
				{
					SeparateUnits(*unit_a, *unit_b);
				}
			}
			++count_b;
			++unit_b;
		}
		++count_a;
		++unit_a;
	}
}

void TacticalAI::SeparateUnits(Unit* unit_a, Unit* unit_b)
{

	//Both units are ATTACKING
	if (unit_a->state == UNIT_ATTACK && unit_b->state == UNIT_ATTACK)
	{
		//Both units share the SAME target
		if (unit_a->GetTarget() == unit_b->GetTarget())
		{
			//Unit B goes in front of Unit A
			SeparateAtkUnits(unit_b, unit_a);

			//[BUG]: when the destination is to close to the enemy the unit doesn't move
		}
		else
		{
			//DIFFERENT target
			//Unit B goes closer to his target
			SeparateAtkUnits(unit_b, unit_b);
		}
	}

}

void TacticalAI::Vision()
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
					/*
						TODO: both share the same vision now. Only send ONE unit to attack depend on HIS vision.
						Iterate friend list searching for enemies
						Iterate enemy list searching for friends
					*/
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
}

bool TacticalAI::OverlapRectangles(const SDL_Rect r1,const SDL_Rect r2)const
{
	if (r1.x + r1.w < r2.x || r1.x > r2.x + r2.w) 
		return false;
	if (r1.y + r1.h < r2.y || r1.y > r2.y + r2.h) 
		return false;

	return true;
}

void TacticalAI::SeparateAtkUnits(Unit* unit, Unit* reference)
{
	fPoint distance(unit->GetTarget()->GetPosition().x - reference->GetPosition().x, unit->GetTarget()->GetPosition().y - reference->GetPosition().y);
	distance.Normalize();
	distance.Scale(COLLISION_DISTANCE * 2);

	iPoint destination(reference->GetPosition().x + distance.x, reference->GetPosition().y + distance.y);

	//No pathfinding is need it. We asume that nothing is between this units
	destination = App->map->WorldToMap(destination.x, destination.y, COLLIDER_MAP);
	iPoint origin = App->map->WorldToMap(unit->GetPosition().x, unit->GetPosition().y, COLLIDER_MAP);

	vector<iPoint> result_path;
	result_path.push_back(origin);
	result_path.push_back(destination);

	unit->SetPath(result_path);
	unit->resolving_collision = true;
}