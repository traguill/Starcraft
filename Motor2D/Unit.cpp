#include "Unit.h"
#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Map.h"
#include "EntityManager.h"
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
	life = u->life;
	type = u->type;
	width = u->width;
	height = u->height;

	//TODO: declare widht & height colliders
	collider.w = width;
	collider.h = width;
}



void Unit::Update(float dt)
{
	switch (state)
	{
	case UNIT_IDLE:
		//Is this really necessary?
		break;
	case UNIT_MOVE:
		Move(dt);
		break;
	case UNIT_ATTACK:
		Attack(dt);
		break;
	case UNIT_DIE:
		//Timer of the animation and delete the unit
		App->entity->RemoveUnit(this);
		break;
	}
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
	{
		SDL_Rect selected1{ 46, 48, 41, 43 };
		App->render->Blit(App->entity->gui_cursor, r.x - 7, r.y + 8, &selected1);
	}
		

	App->render->Blit(&sprite);

}

void Unit::Attack(float dt)
{
	if (target == NULL)
	{
		LOG("I've killed one enemy");
		state = UNIT_IDLE;
		return;
	}

	if (cool_timer >= cool)
	{
		target = target->ApplyDamage(damage);
		cool_timer = 0;
	}
	else
	{
		cool_timer += dt;
	}
}

Unit* Unit::ApplyDamage(uint dmg)
{
	life -= dmg;
	LOG("Life: %i", life);

	if (life < 0)
	{
		LOG("I'm dead!");
		state = UNIT_DIE;
		return NULL;
	}
	else
		return this;
}

void Unit::Move(float dt)
{
	if (has_destination)
	{	
		//Print path
		if (App->entity->debug)
		{
			vector<iPoint>::iterator p_it = path.begin();
			while (p_it != path.end())
			{
				iPoint vec_pos = App->map->MapToWorld((*p_it).x, (*p_it).y, 2);
				App->render->DrawQuad({ vec_pos.x, vec_pos.y, 8, 8 }, 0, 0, 255, 100, true, true);
				p_it++;
			}
		}

		iPoint unit_pos = GetPosition();
		fPoint float_pos; float_pos.x = unit_pos.x, float_pos.y = unit_pos.y;

		float_pos.x += direction.x * speed * 0.016; // 0.016 must be dt, now is only for debugging
		float_pos.y += direction.y * speed * 0.016;

		float_pos.x = roundf(float_pos.x);
		float_pos.y = roundf(float_pos.y);

		unit_pos.x = float_pos.x;
		unit_pos.y = float_pos.y;

		SetPosition(unit_pos.x, unit_pos.y);

		iPoint map_pos = App->map->WorldToMap(unit_pos.x, unit_pos.y, 2);

		iPoint distance(dst_point.x - map_pos.x, dst_point.y - map_pos.y);

		if (distance.x == distance.y == 0 && path.back().x > map_pos.x && path.back().y > map_pos.y)
			SetDirection();

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

void Unit::CenterUnit()
{
	iPoint new_position = GetPosition();
	iPoint map_position = App->map->WorldToMap(new_position.x, new_position.y, 2);
	new_position = App->map->MapToWorld(map_position.x, map_position.y, 2);
	SetPosition(new_position.x, new_position.y);
}

void Unit::SetPath(vector<iPoint> _path)
{	path = _path;
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