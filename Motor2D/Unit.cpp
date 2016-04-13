#include "Unit.h"
#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Map.h"
#include "EntityManager.h"
#include "j1Input.h"
#include "j1Window.h"
#include "j1Pathfinding.h"
#include "math.h"
#include "j1UIManager.h"

Unit::Unit() : Entity()
{

}

Unit::Unit(Unit* u, bool _is_enemy) : Entity()
{
	is_enemy = _is_enemy;

	if (is_enemy)
		sprite.texture = u->auxiliar_texture;
	else
		sprite.texture = u->sprite.texture;

	auxiliar_texture = NULL;

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

	abilities = u->abilities;

	direction.x = 0;
	direction.y = 1;

	//Animations
	up = u->up;
	up.speed = u->walk_anim_speed;
	down = u->down;
	down.speed = u->walk_anim_speed;
	right = u->right;
	right.speed = u->walk_anim_speed;
	left = u->left;
	left.speed = u->walk_anim_speed;
	up_right = u->up_right;
	up_right.speed = u->walk_anim_speed;
	down_right = u->down_right;
	down_right.speed = u->walk_anim_speed;
	up_left = u->up_left;
	up_left.speed = u->walk_anim_speed;
	down_left = u->down_left;
	down_left.speed = u->walk_anim_speed;

	i_up = u->i_up;
	i_up.speed = u->idle_anim_speed;
	i_down = u->i_down;
	i_down.speed = u->idle_anim_speed;
	i_right = u->i_right;
	i_right.speed = u->idle_anim_speed;
	i_left = u->i_left;
	i_left.speed = u->idle_anim_speed;
	i_up_right = u->i_up_right;
	i_up_right.speed = u->idle_anim_speed;
	i_down_right = u->i_down_right;
	i_down_right.speed = u->idle_anim_speed;
	i_up_left = u->i_up_left;
	i_up_left.speed = u->idle_anim_speed;
	i_down_left = u->i_down_left;
	i_down_left.speed = u->idle_anim_speed;

	a_up = u->a_up;
	a_up.speed = u->attack_anim_speed;
	a_down = u->a_down;
	a_down.speed = u->attack_anim_speed;
	a_right = u->a_right;
	a_right.speed = u->attack_anim_speed;
	a_left = u->a_left;
	a_left.speed = u->attack_anim_speed;
	a_up_right = u->a_up_right;
	a_up_right.speed = u->attack_anim_speed;
	a_down_right = u->a_down_right;
	a_down_right.speed = u->attack_anim_speed;
	a_up_left = u->a_up_left;
	a_up_left.speed = u->attack_anim_speed;
	a_down_left = u->a_down_left;
	a_down_left.speed = u->attack_anim_speed;

	//Has to be updated inside update();
	current_animation = &i_down;

	
	collider.w = u->collider.w;
	collider.h = u->collider.h;

	avoid_change_state = false;
}

Unit::~Unit()
{
	Delete();

}

void Unit::Delete()
{
	path.clear();
	target = NULL;
	attacking_units.clear();
	current_animation = NULL;
	App->ui->EraseElement(hp_bar);
	App->ui->EraseElement(prg_bar);
	queue<UNIT_EVENT> empty;
	swap(events, empty);
}


void Unit::Update(float dt)
{
	//Debug code
	if (App->entity->debug)
	{
		App->render->DrawQuad(GetCollider(), 255, 0, 0, 255, false, true);

		//Paint range
		App->render->DrawCircle(logic_pos.x, logic_pos.y, range, 0, 0, 255, 255, true);

		//Paint vision range
		App->render->DrawCircle(logic_pos.x, logic_pos.y, vision, 0, 255, 255, 255, true);
	}


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

	SetAnimation();
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

		//Drawing health bar
		hp_bar->SetLocalPos(GetPosition().x - 8, GetPosition().y + 15);
		App->render->Blit(hp_bar->GetTexture(), GetPosition().x - 8, GetPosition().y + 15, &hp_bar->GetEmptyBar());

		switch (hp_bar->hp_state)
		{
		case EMPTY:
			App->render->Blit(hp_bar->GetTexture(), GetPosition().x - 8, GetPosition().y + 15, &hp_bar->GetEmptyBar());
			break;
		case LOW:
			App->render->Blit(hp_bar->GetTexture(), GetPosition().x - 8, GetPosition().y + 15, &hp_bar->GetLowBar());
			break;
		case MIDDLE:
			App->render->Blit(hp_bar->GetTexture(), GetPosition().x - 8, GetPosition().y + 15, &hp_bar->GetMiddleBar());
			break;
		case FULL:
			App->render->Blit(hp_bar->GetTexture(), GetPosition().x - 8, GetPosition().y + 15, &hp_bar->GetFullBar());
			break;
		}

		//Drawing progress bar
		prg_bar->SetLocalPos(GetPosition().x - 8, GetPosition().y + 20);
		App->render->Blit(prg_bar->GetTexture(), GetPosition().x - 8, GetPosition().y + 20, &prg_bar->GetEmptyBar());

		switch (prg_bar->hp_state)
		{
		case EMPTY:
			App->render->Blit(prg_bar->GetTexture(), GetPosition().x - 8, GetPosition().y + 20, &prg_bar->GetEmptyBar());
			break;
		case LOW:
			App->render->Blit(prg_bar->GetTexture(), GetPosition().x - 8, GetPosition().y + 20, &prg_bar->GetLowBar());
			break;
		case MIDDLE:
			App->render->Blit(prg_bar->GetTexture(), GetPosition().x - 8, GetPosition().y + 20, &prg_bar->GetMiddleBar());
			break;
		case FULL:
			App->render->Blit(prg_bar->GetTexture(), GetPosition().x - 8, GetPosition().y + 20, &prg_bar->GetFullBar());
			break;
		}
	}

	App->render->Blit(&sprite);

}

void Unit::Attack(float dt)
{
	if (target == NULL)
	{
		App->tactical_ai->SetEvent(ENEMY_KILLED, this);
		return;
	}

	//Check the target if it is in range
	if (target->GetPosition().DistanceTo(logic_pos) > range)
	{
		App->tactical_ai->SetEvent(ENEMY_RUNNING, this, target);
		return;
	}

	if (invisible)
		SetVisible();

	if (cool_timer >= cool)
	{
		target->ApplyDamage(damage, this);
		cool_timer = 0;
	}
	else
	{
		cool_timer += dt;
	}
}



void Unit::ApplyDamage(uint dmg,Unit* source)
{
	if (source->state == UNIT_DIE) //Just check this case, erase if never happens
	{
		LOG("A death unit is attacking me");
	}

	//Start attacking if the unit are NOT: death, already attackin or resolving a collision
	if (avoid_change_state == false)
	{
		if (state != UNIT_ATTACK)
		{
			if (state != UNIT_DIE)
			{
				if (is_enemy)
					LOG("Enemy: Someone attacked me! (%d)", source->is_enemy);
				else
					LOG("Friend: Someone attacked me! (%d)", source->is_enemy);
				App->tactical_ai->SetEvent(ATTACKED, this, source);
			}
		}
	}
	

	//ACTUAL MOMENT WHEN THE UNIT ATTACK

	life -= dmg;
	
	//Modifying health bar according to the damage recieved
	
	int dam = (dmg * hp_bar->GetFullBar().w) / hp_bar->current_number;
	int new_len = hp_bar->GetFullBar().w - dam;

	hp_bar->SetBarsLength(new_len);
	hp_bar->current_number = life;

	if (is_enemy)
		LOG("Life (enemy): %i", life);
	else
		LOG("Life (friend): %i", life);

	if (life <= 0)
	{
		if (is_enemy)
			LOG("Enemy: I'm dead!");
		else
			LOG("Friend: I'm dead!");

		//Send attacking units that I'm death
		list<Unit*>::iterator a_unit = attacking_units.begin();
		while (a_unit != attacking_units.end())
		{
			(*a_unit)->target = NULL;
			++a_unit;
		}
		//Send target that I'm death
		if (target != NULL)
		{
			target->attacking_units.remove(this);
		}
		
		

		state = UNIT_DIE;
	}
}

void Unit::Move(float dt)
{
	if (has_destination)
	{	
		if (target != NULL && avoid_change_state == false)
		if (CheckTargetRange() == true)
			return;

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
				//PATH COMPLETED!
				avoid_change_state = false;
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

void Unit::SetAnimation()
{
	if (state == UNIT_ATTACK)
	{
		if (target)
		{
			iPoint target_pos = target->GetPosition();
			iPoint unit_pos = GetPosition();

			direction.x = target_pos.x - unit_pos.x;
			direction.y = target_pos.y - unit_pos.y;

			direction.Normalize();
		}
	}
	
	float angle = atan(direction.y / direction.x) * RADTODEG;

	float section = abs(angle / 45);

	if (direction.x >= 0 && direction.y >= 0)
	{
		if (state == UNIT_MOVE)
		{
			if (section >= 0 && section <= 0.5)
				current_animation = &right;
			else if (section >= 0.5 && section <= 1.5)
				current_animation = &down_right;
			else if (section >= 1.5 && section <= 2)
				current_animation = &down;
		}
		else if (state == UNIT_IDLE)
		{
			if (section >= 0 && section <= 0.5)
				current_animation = &i_right;
			else if (section >= 0.5 && section <= 1.5)
				current_animation = &i_down_right;
			else if (section >= 1.5 && section <= 2)
				current_animation = &i_down;
		}
		else if (state == UNIT_ATTACK)
		{
			if (section >= 0 && section <= 0.5)
				current_animation = &a_right;
			else if (section >= 0.5 && section <= 1.5)
				current_animation = &a_down_right;
			else if (section >= 1.5 && section <= 2)
				current_animation = &a_down;
		}
	}
	else if (direction.x <= 0 && direction.y >= 0)
	{
		if (state == UNIT_MOVE)
		{
			if (section >= 0 && section <= 0.5)
				current_animation = &left;
			else if (section >= 0.5 && section <= 1.5)
				current_animation = &down_left;
			else if (section >= 1.5 && section <= 2)
				current_animation = &down;
		}
		else if (state == UNIT_IDLE)
		{
			if (section >= 0 && section <= 0.5)
				current_animation = &i_left;
			else if (section >= 0.5 && section <= 1.5)
				current_animation = &i_down_left;
			else if (section >= 1.5 && section <= 2)
				current_animation = &i_down;
		}
		else if (state == UNIT_ATTACK)
		{
			if (section >= 0 && section <= 0.5)
				current_animation = &a_left;
			else if (section >= 0.5 && section <= 1.5)
				current_animation = &a_down_left;
			else if (section >= 1.5 && section <= 2)
				current_animation = &a_down;
		}
	}
	else if (direction.x <= 0 && direction.y <= 0)
	{
		if (state == UNIT_MOVE)
		{
			if (section >= 0 && section <= 0.5)
				current_animation = &left;
			else if (section >= 0.5 && section <= 1.5)
				current_animation = &up_left;
			else if (section >= 1.5 && section <= 2)
				current_animation = &up;
		}
		else if (state == UNIT_IDLE)
		{
			if (section >= 0 && section <= 0.5)
				current_animation = &i_left;
			else if (section >= 0.5 && section <= 1.5)
				current_animation = &i_up_left;
			else if (section >= 1.5 && section <= 2)
				current_animation = &i_up;
		}
		else if (state == UNIT_ATTACK)
		{
			if (section >= 0 && section <= 0.5)
				current_animation = &a_left;
			else if (section >= 0.5 && section <= 1.5)
				current_animation = &a_up_left;
			else if (section >= 1.5 && section <= 2)
				current_animation = &a_up;
		}
	}
	else if (direction.x >= 0 && direction.y <= 0)
	{
		if (state == UNIT_MOVE)
		{
			if (section >= 0 && section <= 0.5)
				current_animation = &right;
			else if (section >= 0.5 && section <= 1.5)
				current_animation = &up_right;
			else if (section >= 1.5 && section <= 2)
				current_animation = &up;
		}
		else if (state == UNIT_IDLE)
		{
			if (section >= 0 && section <= 0.5)
				current_animation = &i_right;
			else if (section >= 0.5 && section <= 1.5)
				current_animation = &i_up_right;
			else if (section >= 1.5 && section <= 2)
				current_animation = &i_up;
		}
		else if (state == UNIT_ATTACK)
		{
			if (section >= 0 && section <= 0.5)
				current_animation = &a_right;
			else if (section >= 0.5 && section <= 1.5)
				current_animation = &a_up_right;
			else if (section >= 1.5 && section <= 2)
				current_animation = &a_up;
		}
	}

	//Animations
	sprite.rect.x = current_animation->getCurrentFrame().x;
	sprite.rect.y = current_animation->getCurrentFrame().y;
	
}

UNIT_TYPE Unit::GetType()const
{
	return type;

}

uint Unit::GetRange()const
{
	return range;
}

bool Unit::CheckTargetRange()
{
	bool ret = false;
	if (logic_pos.DistanceTo(target->GetPosition()) <= range) 
	{
		App->tactical_ai->SetEvent(ENEMY_TARGET, this, target);
		ret = true;
	}

	return ret;
}

void Unit::SetTarget(Unit* unit)
{
	if (unit != NULL)
	{
		target = unit;

		unit->attacking_units.push_back(this);
	}
}

Unit* Unit::GetTarget()
{
	return target;
}

void Unit::DiscardTarget()
{
	if (target != NULL)
	{
		target->attacking_units.remove(this); //DO NOT USE THIS METHOD TO DISCARD ALL TARGETS
		target = NULL;
	}
}

void Unit::AddPath(vector<iPoint> _path)
{
	vector<iPoint>::iterator tile = _path.begin();
	while (tile != _path.end())
	{
		path.push_back(*tile);
		++tile;
	}
}

vector<iPoint> Unit::GetPath()const
{
	return path;
}

bool Unit::IsVisible()const
{
	return !invisible;
}

void Unit::UseAbility(uint id)
{
	if (id > abilities.size())
	{
		LOG("Error: unit doesn't have this number of ability");
		return;
	}

	int counter = 1;
	list<UNIT_ABILITY>::iterator ability = abilities.begin();
	while (ability != abilities.end())
	{
		if (counter == id)
		{
			CastAbility(*ability);
			return;
		}
		++counter;
		++ability;
	}

}

void Unit::CastAbility(const UNIT_ABILITY ability)
{
	switch (ability)
	{
	case INVISIBLE:
		Invisibility();
		break;
	case SNIPPER:
		Snipper();
		break;
	case HEAL:
		break;
	}
}

/*
** ABILITIES -------------------------------------------------------------------------------
*/

void Unit::Invisibility()
{
	//Check mana here

	//Check if another ability is in use

	if (state == UNIT_DIE || snipping == true)
		return;

	if (invisible == false)
	{
		sprite.alpha = INVISIBILITY_ALPHA;
		state = UNIT_IDLE;
		invisible = true;

		//Discard the target
		if (target != NULL)
			target->attacking_units.remove(this);

		target = NULL;

		//Tell attaking units to ignore me
		list<Unit*>::iterator atk_unit = attacking_units.begin();
		while (atk_unit != attacking_units.end())
		{
			(*atk_unit)->target = NULL;
			++atk_unit;
		}

		attacking_units.clear();
	}
	else
	{
		//Turn visible
		SetVisible(); 
	}
	
}

void Unit::SetVisible()
{
	invisible = false;
	sprite.alpha = 255;
}

void Unit::Snipper()
{
	if (snipping == true)
	{
		DisableSnipper();
	}


	//Snipper CAN NOT be: death, using invisibility or attacking or already snipping
	if (state == UNIT_DIE || invisible == true || state == UNIT_ATTACK)
		return;

	//snipping = true;
	//Camera transition (search best position)
	avoid_change_state = true;
	state = UNIT_IDLE;
	has_destination = false;
	path.clear();

	iPoint dir(round(direction.x), round(direction.y));

	SDL_Rect cam = App->render->camera;

	//North-West
	if (dir.x == 1 && dir.y == 1)
	{
		App->render->SetTransition(-logic_pos.x, -logic_pos.y);
	}
	//North
	if (dir.x == 0 && dir.y == 1)
	{
		App->render->SetTransition(-logic_pos.x + cam.w / 2, -logic_pos.y);
	}
	//North-East
	if (dir.x == -1 && dir.y == 1)
	{
		App->render->SetTransition(-logic_pos.x + cam.w, -logic_pos.y);
	}
	//East
	if (dir == iPoint(-1, 0) || dir == iPoint(-1, -1))
	{
		App->render->SetTransition(-logic_pos.x + cam.w, -logic_pos.y + cam.h / 2);
	}
	//Up
	if (dir.x == 0 && dir.y == -1)
	{
		App->render->SetTransition(-logic_pos.x + cam.w / 2, -logic_pos.y + 3 * cam.h / 4);
	}
	//West
	if (dir == iPoint(1, 0) || dir == iPoint(1, 1))
	{
		App->render->SetTransition(-logic_pos.x, -logic_pos.y + cam.h / 2);
	}

	App->entity->SNIPPER_MODE = true;
	//Activate bullet time
	App->entity->bullet_time = true;
	//Entity manager to: snipe mode

	//How can I draw snipping animation?


}

void Unit::Shoot(int x, int y)
{

	iPoint origin = App->map->WorldToMap(logic_pos.x, logic_pos.y, COLLIDER_MAP);
	iPoint dst = App->map->WorldToMap(x, y, COLLIDER_MAP);

	iPoint destination(x, y);

	//A wall is in our way
	if (App->pathfinding->CreateLine(origin, dst) == false)
	{
		destination = App->map->MapToWorld(App->pathfinding->GetLineTile().x, App->pathfinding->GetLineTile().y, COLLIDER_MAP);
	}

	//Create bullet [MISS A TEXTURE!]
	Bullet* bullet = new Bullet();
	bullet->SetPosition(logic_pos.x, logic_pos.y);
	bullet->source = this;
	bullet->destination = destination;

	fPoint direction(x - logic_pos.x, y - logic_pos.y);
	direction.Normalize();

	bullet->direction = direction;

	App->entity->bullets.push_back(bullet);

	//Shake Cam
	direction.x = round(direction.x);
	direction.y = round(direction.y);
	direction.Scale(30);
	iPoint cam_initial(App->render->camera.x, App->render->camera.y);
	App->render->camera.x += direction.x;
	App->render->camera.y += direction.y;

	App->render->SetTransition(cam_initial.x, cam_initial.y);

}

void Unit::DisableSnipper()
{
	//END SNIPPING
	snipping = false;
	App->entity->SNIPPER_MODE = false;
	App->entity->bullet_time = false;
	App->entity->actual_bullet_time = 0.0f;
}