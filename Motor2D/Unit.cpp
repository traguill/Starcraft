#include "Unit.h"
#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Map.h"


Unit::Unit() : Entity()
{

}

Unit::Unit(Unit* u) : Entity()
{
	speed = u->speed;
	damage = u->damage;
	vision = u->vision;
	range = u->range;
	cool = u->cool;
	type = u->type;

}



void Unit::Update(float dt)
{
	if (state == UNIT_MOVE)
		Move(dt);
}

void Unit::Draw()
{
	SDL_Rect r;
	r.x = pos.x;
	r.y = pos.y;
	r.w = 24;
	r.h = 32;
	if (selected == true)
		App->render->DrawQuad(r, 0, 255, 0, 255, false, true);

	App->render->Blit(texture, pos.x, pos.y, NULL);
}

void Unit::Move(float dt)
{
	if (has_destination)
	{	
		//Print path
		
			vector<iPoint>::iterator p_it = path.begin();
			while (p_it != path.end())
			{
				iPoint vec_pos = App->map->MapToWorld((*p_it).x, (*p_it).y, 2);
				App->render->DrawQuad({ vec_pos.x, vec_pos.y, 8, 8 }, 0, 0, 255, 255, false, true);
				p_it++;
			}
		
		
		
		pos.x += direction.x * (speed * dt);
		pos.y += direction.y * (speed * dt);

		iPoint map_pos = App->map->WorldToMap(pos.x, pos.y, 2);

		iPoint distance(dst_point.x - map_pos.x, dst_point.y - map_pos.y);

		if (distance.Sign(distance.x) != direction.Sign(direction.x) || distance.Sign(distance.y) != direction.Sign(direction.y))
		{
			GetDirection();
		}
	}
	else
	{
		GetDirection();
	}
}

void Unit::SetPath(vector<iPoint> _path)
{
	path = _path;
	state = UNIT_MOVE;
}

void Unit::GetDirection()
{
	if (path.size() != 0)
	{
		dst_point = path.front();
		path.erase(path.begin());

		iPoint map_pos = App->map->WorldToMap(pos.x, pos.y, 2);

		if (map_pos == dst_point)
		{
			GetDirection();
			return;
		}

		direction.x = dst_point.x - map_pos.x;
		direction.y = dst_point.y - map_pos.y;

		direction.Normalize();

		has_destination = true;
	}
	else
	{
		has_destination = false;
		state = UNIT_IDLE;
	}
}