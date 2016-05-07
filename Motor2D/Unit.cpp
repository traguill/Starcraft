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
#include "j1Audio.h"

Unit::Unit() : Entity()
{
	auxiliar_texture = NULL;
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

	state = UNIT_IDLE;

	abilities = u->abilities;

	direction.x = 0;
	direction.y = 1;

	max_life = u->max_life;
	max_mana = u->max_mana;
	mana = u->mana;
	mana_regen = u->mana_regen;

	//fx
	attack_fx = u->attack_fx;
	death_fx = u->death_fx;

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

	death = u->death;
	death_pos_corrector = u->death_pos_corrector;
	death_size = u->death_size;
	death.speed = u->death_anim_speed;
	death.loop = false;

	//Has to be updated inside update();
	current_animation = &i_down;

	collider.w = u->collider.w;
	collider.h = u->collider.h;

	avoid_change_state = false;

	patrol = false;
}

Unit::~Unit()
{
	Delete();

}

void Unit::Delete()
{
	path.clear();
	patrol_path.clear();
	target = NULL;
	attacking_units.clear();
	current_animation = NULL;

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
		if (patrol)
			SetPath(patrol_path);
		break;
	case UNIT_MOVE:
		Move(dt);
		break;
	case UNIT_ATTACK:
		Attack(dt);
		break;
	case UNIT_DIE:
		//Timer of the animation and delete the unit
		if (death.finished() && (type == MARINE || type == GHOST || type == MEDIC))
			App->entity->RemoveUnit(this);
		else if (type != MARINE && type != GHOST && type != MEDIC)
			App->entity->RemoveUnit(this);
		break;
	}
	//Update mana
	mana += mana_regen * dt;
	if (mana > max_mana) 
		mana = max_mana;

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

	}

	if (state == UNIT_DIE)
	{
		sprite.position = GetDrawPosition() + death_pos_corrector;
		App->render->Blit(&sprite);
	}
	else
	{
		App->render->Blit(&sprite);

		/*if (selected)
			DrawVisionCone();
			*/
	}
		
}

void Unit::DrawVisionCone()
{

	//Lateral Lines
	fPoint left_vision(direction);
	fPoint right_vision(left_vision);
	left_vision.Rotate(M_PI / 6);
	right_vision.Rotate(-M_PI / 6);
	left_vision.Scale(vision);
	right_vision.Scale(vision);

	left_vision.x += logic_pos.x;
	left_vision.y += logic_pos.y;
	right_vision.x += logic_pos.x;
	right_vision.y += logic_pos.y;

	fPoint origin(logic_pos.x, logic_pos.y);
	vector<iPoint> open_points, key_points;

	open_points= CollidersInsideConeVision(origin, right_vision, left_vision);

	GetKeyPointsConeVision(open_points, key_points, origin);

	//Draw key_points
	vector<iPoint>::iterator k = key_points.begin();
	while (k != key_points.end())
	{
		iPoint k_tile = App->map->WorldToMap(k->x, k->y, COLLIDER_MAP);

		vector<iPoint>::iterator k2 = key_points.begin();
		while (k2 != key_points.end())
		{
			iPoint k2_tile = App->map->WorldToMap(k2->x, k2->y, COLLIDER_MAP);
			if (HitAdjacentTile(k_tile, k2_tile) == true)
			{
				App->render->DrawLine(k->x, k->y, k2->x, k2->y, 255, 0, 0, 255, true);
			}
			++k2;
		}
		++k;
	}

	//Actual Draw
	App->render->DrawLine(logic_pos.x, logic_pos.y, left_vision.x,  left_vision.y, 255, 0, 0, 255, true);
	App->render->DrawLine(logic_pos.x, logic_pos.y,  right_vision.x,  right_vision.y, 255, 0, 0, 255, true);

}

void Unit::GetKeyPointsConeVision(vector<iPoint>& points, vector<iPoint>& key_points, const fPoint& origin)
{
	//Iterate all points
	//Create line from point to origin
	//Add point to final points
	//Create line to the end
	//Yes: add end to final points
	//No: add collision point to final points

	key_points.clear();

	iPoint origin_tile = App->map->WorldToMap(origin.x, origin.y, COLLIDER_MAP);

	vector<iPoint>::iterator point = points.begin();
	while (point != points.end())
	{		
		if (App->pathfinding->CreateLineWorld((*point), iPoint(origin.x, origin.y)) == true)
		{
			
			//Calculate end point
			fPoint end((*point).x - origin.x, (*point).y - origin.y);
			end.Normalize();
			end.Scale(vision);
			
			end.x += origin.x;
			end.y += origin.y;

			iPoint end_tile = App->map->WorldToMap(end.x, end.y, COLLIDER_MAP);

			key_points.push_back((*point));

			//Check if something hits the line (point-->end)
			if (App->pathfinding->CreateLineWorld((*point), iPoint(end.x, end.y)) == true)
			{
				//Nothing hits -> Line point-->end
				if (App->pathfinding->IsWalkable(end_tile) == true)
				{
					key_points.push_back(iPoint(end.x, end.y));
					App->render->DrawLine((*point).x, (*point).y, end.x, end.y, 255, 0, 0, 255, true);
				}
			}
			else
			{
				iPoint hit_pos = App->pathfinding->GetLineWorld();
				hit_pos = App->map->WorldToMap(hit_pos.x, hit_pos.y, COLLIDER_MAP);
				iPoint p_tile = App->map->WorldToMap((*point).x, (*point).y, COLLIDER_MAP);
				
				if (HitAdjacentTile(p_tile, hit_pos) == false && App->pathfinding->IsWalkable(hit_pos))
				{
					hit_pos = App->map->MapToWorld(hit_pos.x, hit_pos.y, COLLIDER_MAP);
					
					key_points.push_back(hit_pos);

					App->render->DrawLine((*point).x, (*point).y, hit_pos.x, hit_pos.y, 255, 0, 0, 255, true);
				}
					
			}
		}
		++point;
	}
	
}

bool Unit::HitAdjacentTile(iPoint origin, iPoint hit)
{
	bool ret = false;

	if (hit.x == origin.x + 1 && hit.y == origin.y) return true;
	if (hit.x == origin.x + 1 && hit.y == origin.y + 1) return true;
	if (hit.x == origin.x + 1 && hit.y == origin.y - 1) return true;
	if (hit.x == origin.x && hit.y == origin.y) return true;
	if (hit.x == origin.x && hit.y == origin.y + 1) return true;
	if (hit.x == origin.x && hit.y == origin.y - 1) return true;
	if (hit.x == origin.x - 1 && hit.y == origin.y) return true;
	if (hit.x == origin.x - 1 && hit.y == origin.y + 1) return true;
	if (hit.x == origin.x - 1 && hit.y == origin.y - 1) return true;

	return ret;
}

vector<iPoint> Unit::CollidersInsideConeVision(fPoint p0, fPoint p1, fPoint p2)
{
	vector<iPoint> colliders;

	//Get Up-Left point (START) and down-right point (END) of the cone (transform it to a rectangle)
	iPoint start, end;

	start.x = min(p0.x, min(p1.x, p2.x));
	start.y = min(p0.y, min(p1.y, p2.y));

	end.x = max(p0.x, max(p1.x, p2.x));
	end.y = max(p0.y, max(p1.y, p2.y));

	iPoint start_tile = App->map->WorldToMap(start.x, start.y, COLLIDER_MAP);
	iPoint end_tile = App->map->WorldToMap(end.x, end.y, COLLIDER_MAP);

	for (int x = start_tile.x; x <= end_tile.x; x++)
	{
		for (int y = start_tile.y; y <= end_tile.y; y++)
		{
			iPoint colliding_tile(x, y);
			if (App->pathfinding->IsWalkable(colliding_tile) == false)
			{
				iPoint colliding_position = App->map->MapToWorld(x, y, COLLIDER_MAP);
				colliding_position.x += 4; //Tile correction to center it
				colliding_position.y += 4;

				if (colliding_position.PointInTriangle(p0, p1, p2) == true)
				{
					//Push all the corners
					colliders.push_back(iPoint(colliding_position.x - 4, colliding_position.y - 4));
					colliders.push_back(iPoint(colliding_position.x + 4, colliding_position.y - 4));
					colliders.push_back(iPoint(colliding_position.x - 4, colliding_position.y + 4));
					colliders.push_back(iPoint(colliding_position.x + 4, colliding_position.y + 4));
				}
					
			}
		}
	}

	return colliders;
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



void Unit::ApplyDamage(uint dmg, Unit* source)
{
	if (source->state == UNIT_DIE) //Just check this case, erase if never happens
	{
		LOG("A death unit is attacking me");
	}

	
	

	//ACTUAL MOMENT WHEN THE UNIT ATTACK
	if (source->GetType() != MEDIC)
	{
		life -= dmg;
	}
	else
	{
		life += dmg;
		if (life >= max_life)
		{
			//Stop healing
			life = max_life;
			source->DiscardTarget();
		}
	}

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
		
			//App->tactical_ai->SetEvent(ENEMY_KILLED, (*a_unit), source);
			(*a_unit)->target = NULL;
			++a_unit;
		}
		//Send target that I'm death
		if (target != NULL)
		{
			target->attacking_units.remove(this);
		}
		
		
		avoid_change_state = true;
		state = UNIT_DIE;
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

				if (source->GetType() == MEDIC && is_enemy == false)
				{
					//Ignore attacking
					LOG("A medic is about to heal me!");
					App->audio->PlayFx(source->attack_fx);
				}
				else
					App->tactical_ai->SetEvent(ATTACKED, this, source);
			}
		}
	}
}

void Unit::Move(float dt)
{
	if (waiting_for_path)
	{
		//Need to wait for the path
		if (App->pathfinding->PathFinished(path_id) == true)
		{
			AsignPath(App->pathfinding->GetPath(path_id));
			waiting_for_path = false;
		}
		else
			return;
	}

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
	waiting_for_path = false;
	has_destination = false;
	path = _path;
	state = UNIT_MOVE;
}

void Unit::SetPathId(uint id)
{
	waiting_for_path = true;
	path.clear();
	this->path_id = id;
	has_destination = false;
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

fPoint Unit::GetDirection()const
{
	return direction;
}

void Unit::SetAnimation()
{
	if (state == UNIT_ATTACK)
	{
		App->audio->PlayFx(attack_fx);
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

	//DEATH anim
	if (state == UNIT_DIE)
	{
		App->audio->PlayFx(death_fx);

		sprite.position = GetDrawPosition() + death_pos_corrector;

		sprite.rect.w = death_size.x;
		sprite.rect.h = death_size.y;
		current_animation = &death;
	}

	//Animations
	sprite.rect.x = current_animation->getCurrentFrame(App->entity->bullet_time).x;
	sprite.rect.y = current_animation->getCurrentFrame(App->entity->bullet_time).y;
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

SDL_Texture* Unit::GetAuxiliarTexture() const
{
	return auxiliar_texture;
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
}

/*
** ABILITIES -------------------------------------------------------------------------------
*/

void Unit::AsignPath(vector<iPoint> main_path)
{
	vector<iPoint>::iterator it = main_path.begin();
	while (it != main_path.end())
	{
		path.push_back(iPoint((*it).x + path_offset_x, (*it).y + path_offset_y));
		++it;
	}

	//PATROL
	if (patrol && patrol_path.empty())
	{
		patrol_path = path;
	}
}

int Unit::GetLife()const
{
	return life;
}

int Unit::GetMana()const
{
	return mana;
}

int Unit::GetMaxLife()const
{
	return max_life;
}

int Unit::GetMaxMana()const
{
	return max_mana;
}