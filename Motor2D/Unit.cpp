#include "Unit.h"
#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Map.h"
#include "j1Input.h"
#include "j1Window.h"
#include "math.h"


Unit::Unit() : Entity()
{

}

Unit::Unit(Unit* u) : Entity()
{
	sprite.texture = u->sprite.texture;
	speed = u->speed;
	damage = u->damage;
	vision = u->vision;
	range = u->range;
	cool = u->cool;
	type = u->type;
	width = u->width;
	height = u->height;

	//TODO: declare widht & height colliders
	collider.w = width;
	collider.h = width;
}



void Unit::Update(float dt)
{
	if (state == UNIT_MOVE)
		Move(dt);
}

void Unit::Draw()
{
	iPoint draw_pos = GetDrawPosition();
	SDL_Rect r;
	r.x = draw_pos.x;
	r.y = draw_pos.y;
	r.w = width;
	r.h = height;
	if (selected == true)
		App->render->DrawQuad(r, 0, 255, 0, 255, false, true);

	App->render->Blit(&sprite);

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
				App->render->DrawQuad({ vec_pos.x, vec_pos.y, 8, 8 }, 0, 0, 255, 100, true, true);
				p_it++;
			}

		iPoint unit_pos = GetPosition();
		fPoint float_pos; float_pos.x = unit_pos.x, float_pos.y = unit_pos.y;

		float_pos.x += direction.x * speed * 0.016;
		float_pos.y += direction.y * speed * 0.016;

		float_pos.x = roundf(float_pos.x);
		float_pos.y = roundf(float_pos.y);

		unit_pos.x = float_pos.x;
		unit_pos.y = float_pos.y;

		SetPosition(unit_pos.x, unit_pos.y);

		iPoint map_pos = App->map->WorldToMap(unit_pos.x, unit_pos.y, 2);

		iPoint distance(dst_point.x - map_pos.x, dst_point.y - map_pos.y);

		if (distance.Sign(distance.x) != direction.Sign(direction.x) || distance.Sign(distance.y) != direction.Sign(direction.y))
		{
			SetDirection();
		}
	}
	else
	{
		SetDirection();
	}
}

void Unit::SetPath(vector<iPoint> _path)
{
	path = _path;
	state = UNIT_MOVE;
}

void Unit::SetDirection()
{
	if (path.size() != 0)
	{
		dst_point = path.front();
		path.erase(path.begin());

		iPoint unit_pos = GetPosition();

		iPoint map_pos = App->map->WorldToMap(unit_pos.x, unit_pos.y, 2);

		if (map_pos == dst_point) //Avoid starting tile (direction would be (0,0) )
		{
			SetDirection(); 
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

iPoint Unit::GetDirection()const
{
	iPoint ret(round(direction.x), round(direction.y));

	return ret;

}

UNIT_TYPE Unit::GetType()const
{
	return type;

}