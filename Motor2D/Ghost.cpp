#include "Ghost.h"
#include "EntityManager.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Input.h"
#include "j1Audio.h"
#include "j1Pathfinding.h"
#include "j1Map.h"
#include "GameScene.h"

Ghost::Ghost() : Unit()
{

}

Ghost::Ghost(Unit* unit) : Unit(unit, false)
{
	auxiliar_texture = unit->GetAuxiliarTexture();
	walk_anim_speed = unit->walk_anim_speed;
	idle_anim_speed = unit->idle_anim_speed;
	attack_anim_speed = unit->attack_anim_speed;
	death_anim_speed = unit->death_anim_speed;
}

Ghost::Ghost(Ghost* ghost, bool _is_enemy) : Unit(ghost, _is_enemy)
{

}

Ghost::~Ghost()
{
	//This could cause trouble if there are more than one sniper
	App->entity->SNIPPER_MODE = false;
}

void Ghost::Update(float dt)
{
	Unit::Update(dt);

	if (invisible)
	{
		mana -= App->entity->invisibility_cost * dt;
		if (mana <= 0)
			SetVisible();
	}
}

void Ghost::Attack(float dt)
{
	Unit::Attack(dt);

	if (invisible)
		SetVisible();
}

bool Ghost::GetSnipping() const
{
	return snipping;
}

bool Ghost::HasHit() const
{
	return has_hit;
}

void Ghost::BulletHits()
{
	has_hit = true;
}

void Ghost::CastAbility(const UNIT_ABILITY ability)
{
	switch (ability)
	{
	case INVISIBLE:
		Invisibility();
		break;
	case SNIPPER:
		Snipper();
		break;
	}
}

void Ghost::Invisibility()
{

	//Check if another ability is in use

	if (state == UNIT_DIE || snipping == true)
		return;

	if (invisible == false)
	{
		//Check mana here (not enough mana to start invisibility)
		if (mana < 10)
			return;

		sprite.alpha = INVISIBILITY_ALPHA;
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

void Ghost::SetVisible()
{
	invisible = false;
	sprite.alpha = 255;
}

void Ghost::Snipper()
{
	if (snipping == true)
	{
		DisableSnipper();
	}


	//Snipper CAN NOT be: death, using invisibility or attacking or already snipping
	if (state == UNIT_DIE || invisible == true || state == UNIT_ATTACK)
		return;

	snipping = true;


	//Camera transition (search best position)
	avoid_change_state = true;
	state = UNIT_IDLE;
	has_destination = false;
	path.clear();

	iPoint mouse_pos;
	App->input->GetMouseWorld(mouse_pos.x, mouse_pos.y);

	fPoint dir_f(mouse_pos.x - logic_pos.x, mouse_pos.y - logic_pos.y);
	dir_f.Normalize();
	dir_f.x = round(dir_f.x);
	dir_f.y = round(dir_f.y);
	direction = dir_f;

	iPoint dir(dir_f.x, dir_f.y);

	SDL_Rect cam = App->render->camera;

	//North-West
	if (dir.x == 1 && dir.y == 1)
	{
		App->render->SetTransition(-logic_pos.x, -logic_pos.y, true);
	}
	//North
	if (dir.x == 0 && dir.y == 1)
	{
		App->render->SetTransition(-logic_pos.x + cam.w / 2, -logic_pos.y, true);
	}
	//North-East
	if (dir.x == -1 && dir.y == 1)
	{
		App->render->SetTransition(-logic_pos.x + cam.w, -logic_pos.y, true);
	}
	//East
	if (dir == iPoint(-1, 0) || dir == iPoint(-1, -1))
	{
		App->render->SetTransition(-logic_pos.x + cam.w, -logic_pos.y + cam.h / 2, true);
	}
	//Up
	if (dir.x == 0 && dir.y == -1)
	{
		App->render->SetTransition(-logic_pos.x + cam.w / 2, -logic_pos.y + 3 * cam.h / 4, true);
	}
	//West
	if (dir == iPoint(1, 0) || dir == iPoint(1, 1))
	{
		App->render->SetTransition(-logic_pos.x, -logic_pos.y + cam.h / 2, true);
	}

	App->entity->SNIPPER_MODE = true;
	//Activate bullet time
	App->entity->bullet_time = 0.5f;

	//Entity manager to: snipe mode

	//Sound
	App->audio->PlayFx(App->entity->sound_sniper_mode);


}

void Ghost::Shoot(int x, int y)
{

	if (App->game_scene->sniper_ammo == 0)
	{
		App->game_scene->no_ammo->is_visible = true;
		App->game_scene->sniper_ui_timer.Start();
		return;
	}

	if (mana - App->entity->snipper_cost < 0)
	{
		App->game_scene->no_energy->SetVisible(true);
		App->game_scene->sniper_ui_timer.Start();
		return;
	}

	else
	{
		mana -= App->entity->snipper_cost;
		App->game_scene->sniper_ammo--;
	}
	
	fPoint destination(x - logic_pos.x, y - logic_pos.y);
	destination.Normalize();
	destination.Scale(800);
	destination.x += logic_pos.x;
	destination.y += logic_pos.y;

	iPoint origin = App->map->WorldToMap(logic_pos.x, logic_pos.y, COLLIDER_MAP);
	iPoint dst = App->map->WorldToMap(destination.x, destination.y, COLLIDER_MAP);

	//A wall is in our way
	if (App->pathfinding->CreateLine(origin, dst) == false)
	{
		iPoint result = App->map->MapToWorld(App->pathfinding->GetLineTile().x, App->pathfinding->GetLineTile().y, COLLIDER_MAP);
		destination.x = result.x;
		destination.y = result.y;
	}

	//Create bullet 
	Bullet* bullet = new Bullet(App->entity->db_bullet);
	bullet->SetPosition(logic_pos.x, logic_pos.y);
	bullet->source = this;
	bullet->destination.x = destination.x;
	bullet->destination.y = destination.y;
	bullet->origin = bullet->GetPosition();

	fPoint direction(x - logic_pos.x, y - logic_pos.y);
	direction.Normalize();

	bullet->direction = direction;

	App->entity->bullets.push_back(bullet);

	//Shake Cam
	App->render->lock_camera = false;
	direction.x = round(direction.x);
	direction.y = round(direction.y);
	direction.Scale(30);
	iPoint cam_initial(App->render->camera.x, App->render->camera.y);
	App->render->camera.x += direction.x;
	App->render->camera.y += direction.y;

	App->render->SetTransition(cam_initial.x, cam_initial.y, true);

	//Play sound
	App->audio->PlayFx(App->entity->sound_shoot);
}

void Ghost::DisableSnipper()
{
	//END SNIPPING
	snipping = false;
	App->entity->SNIPPER_MODE = false;
	//Disable bullet mode
	App->entity->bullet_time = 1.0f;
	App->render->lock_camera = false;
	App->render->DiscardTransition();
}