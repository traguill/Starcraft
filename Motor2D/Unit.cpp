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
	sprite.rect.w = u->width;
	sprite.rect.h = u->height;
	speed = u->speed;
	damage = u->damage;
	vision = u->vision;
	range = u->range;
	cool = u->cool;
	life = u->life;
	type = u->type;
	width = u->width;
	height = u->height;

	//Animations
	up = u->up;
	up.speed = u->anim_speed;
	down = u->down;
	down.speed = u->anim_speed;
	right = u->right;
	right.speed = u->anim_speed;
	left = u->left;
	left.speed = u->anim_speed;
	up_right = u->up_right;
	up_right.speed = u->anim_speed;
	down_right = u->down_right;
	down_right.speed = u->anim_speed;
	up_left = u->up_left;
	up_left.speed = u->anim_speed;
	down_left = u->down_left;
	down_left.speed = u->anim_speed;

	//Has to be updated inside update();
	current_animation = &down;

	//TODO: declare widht & height colliders
	collider.w = width;
	collider.h = width;
}

Unit::~Unit()
{

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
	
	//Animations
	sprite.rect.x = current_animation->getCurrentFrame().x;
	sprite.rect.y = current_animation->getCurrentFrame().y;

	App->render->Blit(&sprite);

}

void Unit::Attack(float dt)
{
	if (target == NULL)
	{
		App->tactical_ai->SetEvent(ENEMY_KILLED, this);
		return;
	}

	if (cool_timer >= cool)
	{
		target = target->ApplyDamage(damage, this);
		cool_timer = 0;
	}
	else
	{
		cool_timer += dt;
	}
}



Unit* Unit::ApplyDamage(uint dmg,Unit* source)
{
	
	if (state != UNIT_ATTACK && state != UNIT_DIE)
	{
		LOG("Someone attacked me!");
		App->tactical_ai->SetEvent(ATTACKED, this, source); 
	}

	life -= dmg;
	LOG("Life: %i", life);

	if (life <= 0)
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
		SetDirection();
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

		float_pos.x += (direction.x * speed) * dt;
		float_pos.y += (direction.y * speed) * dt;

		float_pos.x = roundf(float_pos.x);
		float_pos.y = roundf(float_pos.y);

		unit_pos.x = float_pos.x;
		unit_pos.y = float_pos.y;

		SetPosition(unit_pos.x, unit_pos.y);

		iPoint dst_world = App->map->MapToWorld(dst_point.x, dst_point.y, COLLIDER_MAP);

		if (unit_pos.DistanceNoSqrt(dst_world) <= MOVE_RADIUS)
		{
			if (path.size() != 0)
			{
				dst_point = path.front();
				path.erase(path.begin());
				SetDirection();
			}
			else
			{
				has_destination = false;
				App->tactical_ai->SetEvent(END_MOVING, this);
			}
			
		}
	}
	else
	{
		if (path.size() != 0)
		{
			dst_point = path.front();
			path.erase(path.begin());
			SetDirection();
		}
		else
		{
			has_destination = false;
			state = UNIT_IDLE;
		}
	}
}

void Unit::CenterUnit()
{
	iPoint new_position = GetPosition();
	iPoint map_position = App->map->WorldToMap(new_position.x, new_position.y, COLLIDER_MAP);
	new_position = App->map->MapToWorld(map_position.x, map_position.y, COLLIDER_MAP);
	SetPosition(new_position.x, new_position.y);
}

void Unit::SetPath(vector<iPoint> _path)
{	
	has_destination = false;
	path = _path;
	state = UNIT_MOVE;
}

void Unit::SetDirection()
{
		iPoint unit_pos = GetPosition();

		iPoint map_pos = App->map->WorldToMap(unit_pos.x, unit_pos.y, COLLIDER_MAP);

		if (map_pos == dst_point) //Avoid starting tile (direction would be (0,0) )
		{
			if (path.size() > 0)
			{
				dst_point = path.front();
				path.erase(path.begin());
				SetDirection();
			}
			return;
		}

		iPoint dst_world = App->map->MapToWorld(dst_point.x, dst_point.y, COLLIDER_MAP);

		direction.x = dst_world.x - unit_pos.x;
		direction.y = dst_world.y - unit_pos.y;

		direction.Normalize();

		has_destination = true;
	
	
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

uint Unit::GetRange()const
{
	return range;
}