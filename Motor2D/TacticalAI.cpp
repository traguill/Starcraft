#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Textures.h"
#include "j1FileSystem.h"
#include "TacticalAI.h"
#include "EntityManager.h"


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


void TacticalAI::SetEvent(UNIT_EVENT unit_event, Unit* unit, Unit* target){

	switch (unit_event)
	{

	case ATTACKED:
		if (target)
		{
			SetEvent(ENEMY_TARGET, unit, target);
		}
		break;
	case ENEMY_TARGET:
		if (unit->GetPosition().DistanceTo(target->GetPosition()) < unit->GetRange())
		{
			unit->state = UNIT_ATTACK;
		}
		else
		{
			//Unit move to target
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
			SetEvent(event, unit, target);
		}

		break;
	case GO_TO_POINT:
		//Pathfinding
		unit->state = UNIT_MOVE;
		break;
	case ENEMY_KILLED:
		list<Unit*>::iterator i = App->entity->enemy_units.begin();

		while (i != App->entity->enemy_units.end())
		{
			if ((*i)->GetPosition().DistanceTo(unit->GetPosition()) < unit->GetRange())
			{
				SetEvent(ENEMY_TARGET, unit, (*i));
				return;
			}
			++i;
		}

		unit->state = UNIT_IDLE;

		break;

	}


}