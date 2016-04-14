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
	if (actual_time >= checks)
	{
		Vision();

		CheckCollisions();

		actual_time = 0;
	}
	else
	{
		actual_time += dt;
	}

	

	return true;
}


void TacticalAI::SetEvent(UNIT_EVENT unit_event, Unit* unit, Unit* target){

	//Do not change the state if the unit is resolving a collision (high priority)
	if (unit->avoid_change_state == true)
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
		if (target == NULL)
		{
			LOG("Target SHOULDNT BE NULL HERE!");
		}

		if (unit->GetTarget() == NULL)
			unit->SetTarget(target);
		//Now attack another target
		if (unit->GetTarget() != target)
		{
			unit->DiscardTarget();
			unit->SetTarget(target);
		}
		
		//If the target is in range shoot him
		if (unit->GetPosition().DistanceTo(target->GetPosition()) <= unit->GetRange())
		{
			if (unit->is_enemy)
				LOG("(Enemy):  I'm going to attack");
			else
				LOG("(Friend): I'm going to attack");
			
			unit->state = UNIT_ATTACK;
		}
		else
		{
			if (unit->is_enemy)
				LOG("Enemy: I have a target to kill, I'm going to move closer");
			else
				LOG("Friend: I have a target to kill, I'm going to move closer");

			CalculatePath(unit, target);

			if (unit->events.size() == 0)
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
	case ENEMY_RUNNING:
		if (target == NULL || unit->GetTarget() == NULL || target != unit->GetTarget())
		{
			LOG("Enemy running ERROR: target not valid");
		}

		if (unit->is_enemy)
		{
			//Check if someone is near and attack
			if (SearchNearEnemyUnit(unit, App->entity->friendly_units) == false)
			{
				//No one is near, chase my target
				CalculatePath(unit, target);

				unit->AddPath(target->GetPath()); //Combine both paths

				if (unit->events.size() == 0)
					unit->events.push(ENEMY_TARGET);
			}
		}
		else
		{
			if (SearchNearEnemyUnit(unit, App->entity->enemy_units) == false)
			{
				//No one is near, chase my target
				CalculatePath(unit, target);

				unit->AddPath(target->GetPath()); //Combine both paths

				if (unit->events.size() == 0)
					unit->events.push(ENEMY_TARGET);
			}
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
		if ((*i)->state != UNIT_DIE && (*i)->IsVisible() == true)
		{
			if ((*i)->GetPosition().DistanceTo(unit->GetPosition()) < unit->GetRange()) //Change range by vision range and maybe start pathfinding
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
			if ((*unit_a)->avoid_change_state == false && (*unit_b)->avoid_change_state == false) 
			{
				if ((*unit_a)->state == UNIT_DIE || (*unit_b)->state == UNIT_DIE)
				{
					++count_b;
					++unit_b;
					continue;
				}
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
		if (unit_a->GetTarget() == NULL)
		{
			LOG("UNIT A TARGET NULL");
		}
		if (unit_b->GetTarget() == NULL)
		{
			LOG("UNIT B TARTET NULL");
		}
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

		return;
	}

	//UNIT A moves and Unit B idle
	if (unit_a->state == UNIT_MOVE && unit_b->state == UNIT_IDLE)
	{
		SeparateIdleUnits(unit_a, unit_b);
		return;
	}
	if (unit_a->state == UNIT_IDLE && unit_b->state == UNIT_MOVE)
	{
		SeparateIdleUnits(unit_b, unit_a);
		return;
	}

	if (unit_a->state == UNIT_IDLE && unit_b->state == UNIT_IDLE)
	{
		SeparateIdleUnits(unit_a, unit_b, true);
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
			if ((*unit_f)->state != UNIT_DIE && (*unit_e)->state != UNIT_DIE && (*unit_f)->IsVisible() == true && (*unit_e)->IsVisible() == true)
			{
				if ((*unit_f)->GetPosition().DistanceTo((*unit_e)->GetPosition()) <= (*unit_f)->vision)
				{
					/*
						TODO: both share the same vision now. Only send ONE unit to attack depend on HIS vision.
						Iterate friend list searching for enemies
						Iterate enemy list searching for friends
					*/
					if ((*unit_f)->GetTarget() == NULL && (*unit_f)->GetType() != MEDIC) //MEDICS DOESN'T ATTACK
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
	unit->avoid_change_state = true;
}

void TacticalAI::SeparateIdleUnits(Unit* unit_a, Unit* unit_b, bool both_idle)
{
	//Unit B moves to let unit A pass

	iPoint unit_a_pos = unit_a->GetPosition();
	iPoint unit_b_pos = unit_b->GetPosition();

	fPoint direction;
	if (both_idle == true)
	{
		direction.x = (rand() / (double)(RAND_MAX + 1));
		direction.y = (rand() / (double)(RAND_MAX + 1));
	}
	else
		direction = unit_a->direction;

	direction.Normalize();
	direction.Scale(30); //Change it for distance between units [value]
	direction.Rotate(3.14 * 0.5f);

	iPoint final_tile(unit_b_pos.x + direction.x, unit_b_pos.y + direction.y);
	final_tile = App->map->WorldToMap(final_tile.x, final_tile.y, COLLIDER_MAP);
	iPoint origin_tile = unit_b_pos;
	origin_tile = App->map->WorldToMap(origin_tile.x, origin_tile.y, COLLIDER_MAP);

	if (App->pathfinding->IsWalkable(final_tile) == true)
	{
		vector<iPoint>path;
		if (App->pathfinding->CreateOptimizedPath(origin_tile, final_tile, path) == true)
		{
			unit_b->SetPath(path);
			unit_b->avoid_change_state = true;
		}
		else
		{
			//Impossible to create pathfinding for some reason
		}
	}
	else
	{
		//Final tile no walkable find another one (rotate)
	}
}