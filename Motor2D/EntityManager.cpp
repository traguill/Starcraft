#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Input.h"
#include "EntityManager.h"
#include "j1FileSystem.h"
#include "j1Pathfinding.h"
#include "j1Map.h"
#include "j1UIManager.h"
#include "GameScene.h"
#include "j1Audio.h"

j1EntityManager::j1EntityManager() : j1Module()
{
	name.append("entity_manager");
}

// Destructor
j1EntityManager::~j1EntityManager()
{}

// Called before render is available
bool j1EntityManager::Awake(pugi::xml_node& conf)
{
	bool ret = true;

	units_file_path = conf.child("units_path").attribute("value").as_string();

	return ret;
}

// Called before the first frame
bool j1EntityManager::Start()
{
	bool ret = true;

	LoadUnitsInfo();

	debug = false;

	move_rec.x = move_rec.y = move_rec.w = move_rec.h = 0;

	gui_cursor = App->tex->Load("gui/gui_atlas.png");

	//Loading bar texture
	health_bar = App->tex->Load("healthbar.png");

	LoadSounds();

	return ret;
}

// Update all UIManagers
bool j1EntityManager::PreUpdate()
{


	return true;
}

void j1EntityManager::LoadSounds()
{
	sound_shoot = App->audio->LoadFx("sounds/shoot.ogg");
	sound_sniper_mode = App->audio->LoadFx("sounds/sniper_mode.ogg");
}

bool j1EntityManager::Update(float dt)
{

	//Create units
	//DEBUG-----------------------------------------------------------------------------
	/*if (App->input->GetKey(SDL_SCANCODE_G) == KEY_DOWN)
	{
		LOG("Ghost created");
		iPoint p;  App->input->GetMouseWorld(p.x, p.y);
		CreateUnit(GHOST, p.x, p.y, false);
	}

	if (App->input->GetKey(SDL_SCANCODE_M) == KEY_DOWN)
	{
		LOG("MAarine created");
		iPoint p;  App->input->GetMouseWorld(p.x, p.y);
		CreateUnit(MARINE, p.x, p.y, false);
	}

	if (App->input->GetKey(SDL_SCANCODE_N) == KEY_DOWN)
	{
		LOG("Enemy Marine created");
		iPoint p;  App->input->GetMouseWorld(p.x, p.y);
		CreateUnit(MARINE, p.x, p.y, true);
	}

	if (App->input->GetKey(SDL_SCANCODE_F) == KEY_DOWN)
	{
		LOG("Firebat created");
		iPoint p;  App->input->GetMouseWorld(p.x, p.y);
		CreateUnit(FIREBAT, p.x, p.y, false);
	}

	if (App->input->GetKey(SDL_SCANCODE_O) == KEY_DOWN)
	{
		LOG("Observer created");
		iPoint p;  App->input->GetMouseWorld(p.x, p.y);
		CreateUnit(OBSERVER, p.x, p.y, false);
	}
	if (App->input->GetKey(SDL_SCANCODE_L) == KEY_DOWN)
	{
		LOG("Medic created");
		iPoint p;  App->input->GetMouseWorld(p.x, p.y);
		CreateUnit(MEDIC, p.x, p.y, false);

	}
	*/
	/*if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;
		*/
	//------------------------------------------------------------------------------

	//Basic logic

	if (SNIPPER_MODE == true)
	{
		//Draw line
		iPoint mouse;
		App->input->GetMouseWorld(mouse.x, mouse.y);
		if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN || App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_REPEAT)
		{
			iPoint base = (*selected_units.begin())->GetPosition();
			
			App->render->DrawLine(base.x, base.y, mouse.x, mouse.y, 255, 0, 0, 100, true);
		}
		
		if (App->input->GetKey(SDL_SCANCODE_W) == KEY_UP)
		{
			//DISABLE snipper mode
			selected_units.front()->DisableSnipper();
		}

		if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_UP)
		{
			//Shoot
			selected_units.front()->Shoot(mouse.x, mouse.y);
		}
	}
	else
	{
		SelectUnits();

		if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_UP)
		{
			CheckUnderCursor(); //Checks whats under the cursor position (enemy->attack, nothing->move)
		}

		ActivateAbilities();
	}
	

	//UPDATE UNITS------------------------------------------------------------------------------------
	list<Unit*>::iterator it = friendly_units.begin();
	while (it != friendly_units.end())
	{
		if (App->game_scene->GamePaused() == false)
			(*it)->Update(dt * bullet_time);

		(*it)->Draw();
		it++;
	}

	list<Unit*>::iterator i = enemy_units.begin();
	while (i != enemy_units.end())
	{
		if (App->game_scene->GamePaused() == false)
			(*i)->Update(dt * bullet_time);

		(*i)->Draw();
		i++;
	}


	//Update bullets
	list<Bullet*>::iterator bullet = bullets.begin();
	while (bullet != bullets.end())
	{
		if (App->game_scene->GamePaused() == false)
			(*bullet)->Update(dt*bullet_time);

		(*bullet)->Draw();
		++bullet;
	}
	

	return true;

}

// Called after all Updates
bool j1EntityManager::PostUpdate()
{


	if (units_to_remove.size() > 0)
	{


		LOG("(Manager): Some units need to be destroyed:    %d", units_to_remove.size());
		LOG("(Friendly)Total units: %d, (Enemy)Total units: %d, (Selected): Total units %d", friendly_units.size(), enemy_units.size(), selected_units.size());
		list<Unit*>::iterator i = units_to_remove.begin();

		while (i != units_to_remove.end())
		{
			list<Unit*>::iterator unit_to_remove = i;
			++i;
			DestroyUnit((*unit_to_remove));
		}

		units_to_remove.clear();
		LOG("(Friendly)Total units: %d, (Enemy)Total units: %d, (Selected): Total units %d", friendly_units.size(), enemy_units.size(), selected_units.size());
	}

	if (bullets_to_remove.size() > 0)
	{
		list<Bullet*>::iterator b = bullets_to_remove.begin();
		while (b != bullets_to_remove.end())
		{
			list<Bullet*>::iterator bullet_to_remove = b;
			++b;
			DestroyBullet((*bullet_to_remove));
		}
	}
	bullets_to_remove.clear();
	return true;
}

bool j1EntityManager::CleanUp()
{
	LOG("Freeing Entity Manager");
	map<string, Unit*>::iterator it_db = units_database.begin();
	while (it_db != units_database.end())
	{
		delete it_db->second;
		++it_db;
	}
	units_database.clear();

	list<Unit*>::iterator it_fu = friendly_units.begin();
	while (it_fu != friendly_units.end())
	{
		delete *it_fu;
		++it_fu;
	}
	friendly_units.clear();

	list<Unit*>::iterator it_en = enemy_units.begin();
	while (it_en != enemy_units.end())
	{
		delete *it_en;
		++it_en;
	}
	enemy_units.clear();

	list<Bullet*>::iterator bullet = bullets.begin();
	while (bullet != bullets.end())
	{
		delete (*bullet);
		++bullet;
	}
	bullets.clear();

	App->tex->UnLoad(gui_cursor);
	App->tex->UnLoad(health_bar);

	if (db_bullet != NULL)
	{
		App->tex->UnLoad(db_bullet->sprite.texture);
		delete db_bullet;
		db_bullet = NULL;
	}

	return true;
}

void j1EntityManager::RemoveUnit(Unit* _unit)
{
	if (_unit != NULL)
		units_to_remove.push_back(_unit);
}

void j1EntityManager::DestroyUnit(Unit* _unit)
{
	//Remove the unit in the following lists if exists: friendly, enemy, selected
	//Remove the pointer

	//Destroy from friendly units
	list<Unit*>::iterator f_unit = friendly_units.begin();

	while (f_unit != friendly_units.end())
	{
		if (*f_unit == _unit)
		{
			f_unit = friendly_units.erase(f_unit);
			f_unit--;

			list<Unit*>::iterator s_unit = selected_units.begin();
			for (uint i = 0; s_unit != selected_units.end(); i++, ++s_unit)
			{
				if (*s_unit == _unit)
				{
					if (selected_units.size() > 1)
						App->ui->DeleteMiniWIreframe(i);
					else
						App->ui->OcultWireframes();

					s_unit = selected_units.erase(s_unit);
					s_unit--;
					delete _unit;
					return;
				}
			}
			delete _unit;
			return;
		}
		++f_unit;
	}

	//Destroy from enemy units
	list<Unit*>::iterator e_unit = enemy_units.begin();

	while (e_unit != enemy_units.end())
	{
		if (*e_unit == _unit)
		{
			enemy_units.erase(e_unit);
			delete _unit;
			return;
		}
		++e_unit;
	}

}

bool j1EntityManager::LoadUnitsInfo()
{
	bool ret = true;

	pugi::xml_document	unit_file;
	pugi::xml_node		units;

	char* buf;
	int size = App->fs->Load(units_file_path.c_str(), &buf);
	pugi::xml_parse_result result = unit_file.load_buffer(buf, size);
	RELEASE(buf);

	if (result == NULL)
	{
		LOG("Could not load xml file %s. PUGI error: &s", units_file_path.c_str(), result.description());
		return false;
	}
	else
		units = unit_file.child("units");

	//Abilities cost
	invisibility_cost = units.child("invisibility").attribute("cost").as_float();
	snipper_cost = units.child("snipper").attribute("cost").as_int();

	//Bullet
	pugi::xml_node bul = units.child("bullet");

	db_bullet = new Bullet();

	db_bullet->sprite.texture = App->tex->Load(bul.child("texture").attribute("value").as_string());
	db_bullet->sprite.rect.w = bul.child("width").attribute("value").as_int();
	db_bullet->sprite.rect.h = bul.child("height").attribute("value").as_int();

	db_bullet->pos_up.x = bul.child("up").child("pos").attribute("x").as_int();
	db_bullet->pos_up.y = bul.child("up").child("pos").attribute("y").as_int();

	db_bullet->pos_down.x = bul.child("down").child("pos").attribute("x").as_int();
	db_bullet->pos_down.y = bul.child("down").child("pos").attribute("y").as_int();

	db_bullet->pos_right.x = bul.child("right").child("pos").attribute("x").as_int();
	db_bullet->pos_right.y = bul.child("right").child("pos").attribute("y").as_int();

	db_bullet->pos_left.x = bul.child("left").child("pos").attribute("x").as_int();
	db_bullet->pos_left.y = bul.child("left").child("pos").attribute("y").as_int();

	db_bullet->pos_up_right.x = bul.child("upright").child("pos").attribute("x").as_int();
	db_bullet->pos_up_right.y = bul.child("upright").child("pos").attribute("y").as_int();

	db_bullet->pos_down_right.x = bul.child("downright").child("pos").attribute("x").as_int();
	db_bullet->pos_down_right.y = bul.child("downright").child("pos").attribute("y").as_int();

	db_bullet->pos_up_left.x = bul.child("upleft").child("pos").attribute("x").as_int();
	db_bullet->pos_up_left.y = bul.child("upleft").child("pos").attribute("y").as_int();

	db_bullet->pos_down_left.x = bul.child("downleft").child("pos").attribute("x").as_int();
	db_bullet->pos_down_left.y = bul.child("downleft").child("pos").attribute("y").as_int();

	//UNITS
	pugi::xml_node unit;
	for (unit = units.child("unit"); unit && ret; unit = unit.next_sibling("unit"))
	{
		Unit* unit_db = new Unit();
		unit_db->sprite.texture = App->tex->Load(unit.child("texture_path").attribute("value").as_string());
		unit_db->auxiliar_texture = App->tex->Load(unit.child("auxiliar_texture").attribute("value").as_string());
		unit_db->life = unit.child("life").attribute("value").as_int();
		unit_db->speed = unit.child("speed").attribute("value").as_int();
		unit_db->damage = unit.child("damage").attribute("value").as_int();
		unit_db->vision = unit.child("vision").attribute("value").as_int();
		unit_db->range = unit.child("range").attribute("value").as_int();
		unit_db->cool = unit.child("cool").attribute("value").as_int();
		unit_db->type = UnitTypeToEnum(unit.attribute("TYPE").as_string());
		unit_db->width = unit.child("width").attribute("value").as_int();
		unit_db->height = unit.child("height").attribute("value").as_int();
		unit_db->collider.w = unit.child("collider").attribute("width").as_int();
		unit_db->collider.h = unit.child("collider").attribute("height").as_int();
		unit_db->mana = unit.child("mana").attribute("value").as_int();
		unit_db->mana_regen = unit.child("mana_regen").attribute("value").as_int();
		unit_db->max_life = unit_db->life;
		unit_db->max_mana = unit_db->mana;
		
		string at_fx = unit.child("attack_fx").attribute("value").as_string();
		string dth_fx = unit.child("death_fx").attribute("value").as_string();

		unit_db->attack_fx = App->audio->LoadFx(at_fx.c_str());
		unit_db->death_fx = App->audio->LoadFx(dth_fx.c_str());

		//Abilities check if the unit has any
		if (unit.child("abilities").attribute("value").as_bool() == true)
		{
			pugi::xml_node ability;
			for (ability = unit.child("abilities").child("ability"); ability; ability = ability.next_sibling("ability"))
			{
				string invisible = "INVISIBLE";
				string snipper = "SNIPPER";
				string heal = "HEAL";

				if (ability.attribute("value").as_string() == invisible)
				{
					unit_db->abilities.push_back(INVISIBLE);
				}
				if (ability.attribute("value").as_string() == snipper)
				{
					unit_db->abilities.push_back(SNIPPER);
				}
				if (ability.attribute("value").as_string() == heal)
				{
					unit_db->abilities.push_back(HEAL);
				}
			}
		}

		//Animations
		unit_db->up.frames.clear();
		unit_db->down.frames.clear();
		unit_db->right.frames.clear();
		unit_db->left.frames.clear();
		unit_db->up_right.frames.clear();
		unit_db->down_right.frames.clear();
		unit_db->up_left.frames.clear();
		unit_db->down_left.frames.clear();

		unit_db->i_up.frames.clear();
		unit_db->i_down.frames.clear();
		unit_db->i_right.frames.clear();
		unit_db->i_left.frames.clear();
		unit_db->i_up_right.frames.clear();
		unit_db->i_down_right.frames.clear();
		unit_db->i_up_left.frames.clear();
		unit_db->i_down_left.frames.clear();

		unit_db->a_up.frames.clear();
		unit_db->a_down.frames.clear();
		unit_db->a_right.frames.clear();
		unit_db->a_left.frames.clear();
		unit_db->a_up_right.frames.clear();
		unit_db->a_down_right.frames.clear();
		unit_db->a_up_left.frames.clear();
		unit_db->a_down_left.frames.clear();

		unit_db->death.frames.clear();

		//move
		for (pugi::xml_node rect = unit.child("up").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->up.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("down").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->down.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("right").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->right.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("left").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->left.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("upright").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->up_right.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("downright").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->down_right.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("upleft").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->up_left.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("downleft").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->down_left.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		//idle
		for (pugi::xml_node rect = unit.child("iup").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->i_up.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("idown").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->i_down.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("iright").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->i_right.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("ileft").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->i_left.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("iupright").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->i_up_right.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("idownright").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->i_down_right.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("iupleft").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->i_up_left.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("idownleft").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->i_down_left.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		//attack
		for (pugi::xml_node rect = unit.child("aup").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->a_up.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("adown").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->a_down.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("aright").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->a_right.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("aleft").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->a_left.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("aupright").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->a_up_right.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("adownright").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->a_down_right.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("aupleft").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->a_up_left.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}
		for (pugi::xml_node rect = unit.child("adownleft").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->a_down_left.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->width, unit_db->height });
		}

		//DEATH
		unit_db->death_pos_corrector = { unit.child("death").child("pos").attribute("x").as_int(), unit.child("death").child("pos").attribute("y").as_int() };
		unit_db->death_size = { unit.child("death").child("size").attribute("w").as_int(), unit.child("death").child("size").attribute("h").as_int() };
		for (pugi::xml_node rect = unit.child("death").child("rect"); rect; rect = rect.next_sibling("rect"))
		{
			unit_db->death.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), unit_db->death_size.x, unit_db->death_size.y });
		}

		

		//FIREBATATTACK
		//projectiles, gotta implement everything except UP
		if (unit_db->type == FIREBAT)
		{
			pugi::xml_node projectile = unit.child("projectile");

			Projectile* p = new Projectile();

			p->sprite.texture = App->tex->Load(projectile.child("path").attribute("value").as_string());

			p->anim_speed = projectile.child("anim_speed").attribute("value").as_float();

			p->sprite.rect.w = projectile.child("width").attribute("value").as_int();
			p->sprite.rect.h = projectile.child("height").attribute("value").as_int();

			//positions
			p->pos_up.x = projectile.child("up").child("pos").attribute("x").as_int();
			p->pos_up.y = projectile.child("up").child("pos").attribute("y").as_int();

			p->pos_down.x = projectile.child("down").child("pos").attribute("x").as_int();
			p->pos_down.y = projectile.child("down").child("pos").attribute("y").as_int();

			p->pos_right.x = projectile.child("right").child("pos").attribute("x").as_int();
			p->pos_right.y = projectile.child("right").child("pos").attribute("y").as_int();

			p->pos_left.x = projectile.child("left").child("pos").attribute("x").as_int();
			p->pos_left.y = projectile.child("left").child("pos").attribute("y").as_int();

			p->pos_up_right.x = projectile.child("upright").child("pos").attribute("x").as_int();
			p->pos_up_right.y = projectile.child("upright").child("pos").attribute("y").as_int();

			p->pos_down_right.x = projectile.child("downright").child("pos").attribute("x").as_int();
			p->pos_down_right.y = projectile.child("downright").child("pos").attribute("y").as_int();

			p->pos_up_left.x = projectile.child("upleft").child("pos").attribute("x").as_int();
			p->pos_up_left.y = projectile.child("upleft").child("pos").attribute("y").as_int();

			p->pos_down_left.x = projectile.child("downleft").child("pos").attribute("x").as_int();
			p->pos_down_left.y = projectile.child("downleft").child("pos").attribute("y").as_int();

			//animations
			p->up.frames.clear();
			for (pugi::xml_node rect = projectile.child("up").child("rect"); rect; rect = rect.next_sibling("rect"))
			{
				p->up.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), p->sprite.rect.w, p->sprite.rect.h });
			}
			p->down.frames.clear();
			for (pugi::xml_node rect = projectile.child("down").child("rect"); rect; rect = rect.next_sibling("rect"))
			{
				p->down.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), p->sprite.rect.w, p->sprite.rect.h });
			}
			p->right.frames.clear();
			for (pugi::xml_node rect = projectile.child("right").child("rect"); rect; rect = rect.next_sibling("rect"))
			{
				p->right.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), p->sprite.rect.w, p->sprite.rect.h });
			}
			p->left.frames.clear();
			for (pugi::xml_node rect = projectile.child("left").child("rect"); rect; rect = rect.next_sibling("rect"))
			{
				p->left.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), p->sprite.rect.w, p->sprite.rect.h });
			}
			p->up_right.frames.clear();
			for (pugi::xml_node rect = projectile.child("upright").child("rect"); rect; rect = rect.next_sibling("rect"))
			{
				p->up_right.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), p->sprite.rect.w, p->sprite.rect.h });
			}
			p->down_right.frames.clear();
			for (pugi::xml_node rect = projectile.child("downright").child("rect"); rect; rect = rect.next_sibling("rect"))
			{
				p->down_right.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), p->sprite.rect.w, p->sprite.rect.h });
			}
			p->up_left.frames.clear();
			for (pugi::xml_node rect = projectile.child("upleft").child("rect"); rect; rect = rect.next_sibling("rect"))
			{
				p->up_left.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), p->sprite.rect.w, p->sprite.rect.h });
			}
			p->down_left.frames.clear();
			for (pugi::xml_node rect = projectile.child("downleft").child("rect"); rect; rect = rect.next_sibling("rect"))
			{
				p->down_left.frames.push_back({ rect.attribute("x").as_int(), rect.attribute("y").as_int(), p->sprite.rect.w, p->sprite.rect.h });
			}

			unit_db->p = p;

			delete p;
			p = NULL;
		}

		unit_db->walk_anim_speed = unit.child("animwalkspeed").attribute("value").as_float();
		unit_db->idle_anim_speed = unit.child("animidlespeed").attribute("value").as_float();
		unit_db->attack_anim_speed = unit.child("animattackspeed").attribute("value").as_float();
		unit_db->death_anim_speed = unit.child("deathanimspeed").attribute("value").as_float();

		units_database.insert(pair<string, Unit*>(unit.attribute("TYPE").as_string(), unit_db));
	}

	//Print all database (DEBUG)
	PrintUnitDatabase();

	return ret;
}

void j1EntityManager::PrintUnitDatabase()const
{
	map<string, Unit*>::const_iterator i = units_database.begin();

	while (i != units_database.end())
	{

		LOG("UNIT: %s --------------------", (*i).first.c_str());

		LOG("Width: %i, Height: %i", (*i).second->width, (*i).second->height);
		LOG("Life: %i", (*i).second->life);
		LOG("Speed: %i", (*i).second->speed);
		LOG("Damage: %i", (*i).second->damage);
		LOG("Vision: %i", (*i).second->vision);
		LOG("Range: %i", (*i).second->range);
		LOG("Cool: %i", (*i).second->cool);
		LOG("Collider width %i - height %i", (*i).second->collider.w, (*i).second->collider.h);

		LOG("Number of abilities %i", (*i).second->abilities.size());
		LOG("Mana: %i", (*i).second->mana);
		LOG("Mana regeneration: %i", (*i).second->mana_regen);

		++i;
	}
}

string j1EntityManager::UnitTypeToString(UNIT_TYPE type)const
{
	if (type == MARINE) return "MARINE";
	if (type == FIREBAT) return "FIREBAT";
	if (type == GHOST) return "GHOST";
	if (type == MEDIC) return "MEDIC";
	if (type == OBSERVER) return "OBSERVER";
	if (type == ENGINEER) return "ENGINEER";
	if (type == SHIP) return "SHIP";
	if (type == GOLIATH) return "GOLIATH";
	if (type == TANK) return "TANK";
	if (type == VALKYRIE) return "VALKYRIE";

	return NULL;
}
UNIT_TYPE j1EntityManager::UnitTypeToEnum(string type)const
{
	if (type == "MARINE") return MARINE;
	if (type == "FIREBAT") return FIREBAT;
	if (type == "GHOST") return GHOST;
	if (type == "MEDIC") return MEDIC;
	if (type == "OBSERVER") return OBSERVER;
	if (type == "ENGINEER") return ENGINEER;
	if (type == "SHIP") return SHIP;
	if (type == "GOLIATH") return GOLIATH;
	if (type == "TANK") return TANK;
	if (type == "VALKYRIE") return VALKYRIE;

	return MARINE; //Should return empty type
}

void j1EntityManager::SelectUnits()
{
	iPoint mouse_pos;
	App->input->GetMouseWorld(mouse_pos.x, mouse_pos.y);

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		list<Unit*>::iterator it = selected_units.begin();
		list<UIImage*>::iterator wire_it = App->ui->mini_wireframes.begin();

		while (it != selected_units.end())
		{
			(*it)->selected = false;

			if (App->ui->mini_wireframes.size() > 0)
				RELEASE(*wire_it);

			it++; wire_it++;
		}

		App->ui->OcultWireframes();
		selected_units.clear();
		App->ui->mini_wireframes.clear();
		App->input->GetMouseWorld(select_start.x, select_start.y);
	}

	list<Unit*>::iterator friendly_it = friendly_units.begin();
	while (friendly_it != friendly_units.end())
	{
		if ((*friendly_it)->GetPosition().x <= mouse_pos.x && mouse_pos.x <= (*friendly_it)->GetPosition().x + (*friendly_it)->sprite.rect.w && (*friendly_it)->GetPosition().y + 5 >= mouse_pos.y && mouse_pos.y >= (*friendly_it)->GetPosition().y - (*friendly_it)->sprite.rect.h + 5)
		{
			if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
			{
				selected_units.push_back((*friendly_it));
				(*friendly_it)->selected = true;
			}
			App->ui->cursor_state = ON_FRIENDLY;
			break;
		}
		else
			App->ui->cursor_state = STANDARD;
		friendly_it++;
	}

	list<Unit*>::iterator enemy_it = enemy_units.begin();
	while (enemy_it != enemy_units.end())
	{
		if ((*enemy_it)->GetPosition().x <= mouse_pos.x && mouse_pos.x <= (*enemy_it)->GetPosition().x + (*enemy_it)->sprite.rect.w && (*enemy_it)->GetPosition().y + 5 >= mouse_pos.y && mouse_pos.y >= (*enemy_it)->GetPosition().y - (*enemy_it)->sprite.rect.h + 5)
		{
			App->ui->cursor_state = ON_ENEMY;
			break;
		}
		else
		{
			if (App->ui->cursor_state != ON_FRIENDLY)
				App->ui->cursor_state = STANDARD;
		}
		enemy_it++;
	}


	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP)
	{
		App->input->GetMouseWorld(select_end.x, select_end.y);

		(select_start.x < select_end.x) ? selection_rect.x = select_start.x : selection_rect.x = select_end.x;
		(select_start.y < select_end.y) ? selection_rect.y = select_start.y : selection_rect.y = select_end.y;
		iPoint down_right;
		(select_start.x > select_end.x) ? down_right.x = select_start.x : down_right.x = select_end.x;
		(select_start.y > select_end.y) ? down_right.y = select_start.y : down_right.y = select_end.y;

		selection_rect.w = down_right.x - selection_rect.x;
		selection_rect.h = down_right.y - selection_rect.y;

		list<Unit*>::iterator it = friendly_units.begin();
		while (it != friendly_units.end())
		{
			if ((*it)->GetPosition().PointInRect(selection_rect.x, selection_rect.y, selection_rect.w, selection_rect.h) == true)
			{
				selected_units.push_back((*it));
				(*it)->selected = true;
			}
			it++;
		}

		if (selected_units.size() > 1)
		{
			list<Unit*>::iterator select_it = selected_units.begin();
			for (uint i = 0; select_it != selected_units.end(); i++, select_it++)
			{
				App->ui->CreateMiniWireframe((*select_it)->type, i);
			}
		}
	}
}

void j1EntityManager::SetMovement()
{
	if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_UP)
	{
		if (selected_units.size() > 0)
		{
			CalculateMovementRect();

			int mouse_x, mouse_y;
			App->input->GetMouseWorld(mouse_x, mouse_y);
			iPoint destination(App->map->WorldToMap(mouse_x, mouse_y, 2));
			LOG("X: %i", destination.x);
			LOG("Y: %i", destination.y);
			iPoint center_map = App->map->WorldToMap(center.x, center.y, 2);

			vector<iPoint> path;
			if (App->pathfinding->CreateLine(center_map, destination) == false)
			{
				//Create pathfinding;

				path.clear();

				//If you have some units selected & central point is not walkable--------------------------------
				if (selected_units.size() > 1 && App->pathfinding->IsWalkable(center_map) == false)
				{
					list<Unit*>::iterator unit_p = selected_units.begin();
					while (unit_p != selected_units.end())
					{
						iPoint unit_pos = (*unit_p)->GetPosition();
						iPoint unit_map_pos = App->map->WorldToMap(unit_pos.x, unit_pos.y, 2);

						if (App->pathfinding->CreatePath(unit_map_pos, destination) != -1)
							AssignPath(*unit_p, *App->pathfinding->GetLastPath(), NULL);

						++unit_p;
					}
					return;
				}
				//-----------------------------------------------------------------------------------------------

				else if (App->pathfinding->CreatePath(center_map, destination) == -1)

				{
					LOG("Impossible to create path");
					return;
				}

				path = *App->pathfinding->GetLastPath();
			}

			else
			{
				//Only one path
				path.push_back(center_map);
				path.push_back(destination);
			}

			//Assign to each unit its path
			list<Unit*>::iterator unit_p = selected_units.begin();
			while (unit_p != selected_units.end())
			{
				if ((*unit_p)->state == UNIT_DIE)
				{
					++unit_p;
					continue;
				}

				iPoint unit_pos = (*unit_p)->GetPosition();
				iPoint unit_map_pos = App->map->WorldToMap(unit_pos.x, unit_pos.y, 2);
				iPoint rect_offset = unit_map_pos - center_map;
				iPoint end_point = path.back() + rect_offset;

				//Check if there is any collider in the path (copy of the center point of rect path)
				if (App->pathfinding->CreateLine(unit_map_pos, end_point) == true)
					AssignPath(*unit_p, path, &center_map);

				else
				{
					//Check if the destination + rect_offset is valid
					if (App->pathfinding->IsWalkable(end_point) == true)
					{
						//If it is, create a path to go there
						if (App->pathfinding->CreatePath(unit_map_pos, end_point) != 1)
							AssignPath(*unit_p, *App->pathfinding->GetLastPath(), NULL);
					}

					else
					{
						//If it doesn't, go to the mouse point
						if (App->pathfinding->CreatePath(unit_map_pos, destination) != -1)
							AssignPath(*unit_p, *App->pathfinding->GetLastPath(), NULL);
					}
				}
				++unit_p;
			}
		}
	}
}

void j1EntityManager::AssignPath(Unit* unit, vector<iPoint> path, iPoint* center)
{
	iPoint unit_pos = unit->GetPosition();
	iPoint unit_pos_tile(App->map->WorldToMap(unit_pos.x, unit_pos.y, 2));
	iPoint dst_center(0, 0);

	if (center != NULL)
	{
		dst_center.x = unit_pos_tile.x - center->x;
		dst_center.y = unit_pos_tile.y - center->y;
	}

	vector<iPoint> unit_path;
	vector<iPoint>::iterator path_it = path.begin();
	while (path_it != path.end())
	{
		unit_path.push_back(iPoint(path_it->x + dst_center.x, path_it->y + dst_center.y));
		++path_it;
	}

	unit->DiscardTarget();
	unit->avoid_change_state = true;
	unit->SetPath(unit_path);
	unit->CenterUnit();
}

void j1EntityManager::CalculateMovementRect()//
{
	//Values to create rectangle
	int min_x, max_x, min_y, max_y;
	min_x = max_x = min_y = max_y = -1;

	list<Unit*>::iterator it = selected_units.begin();

	while (it != selected_units.end())
	{
		iPoint unit_pos = (*it)->GetPosition();
		//First time
		if (max_x == -1)
		{
			min_x = max_x = unit_pos.x;
			min_y = max_y = unit_pos.y;
		}

		if (unit_pos.x < min_x)  min_x = unit_pos.x;
		if (unit_pos.x > max_x)  max_x = unit_pos.x;
		if (unit_pos.y < min_y)  min_y = unit_pos.y;
		if (unit_pos.y > max_y)  max_y = unit_pos.y;

		++it;
	}

	move_rec.x = min_x;
	move_rec.y = min_y;
	move_rec.w = max_x - min_x;
	move_rec.h = max_y - min_y;

	center.x = move_rec.x + (move_rec.w / 2);
	center.y = move_rec.y + (move_rec.h / 2);
}

void j1EntityManager::CheckUnderCursor()
{
	int mouse_x, mouse_y;
	App->input->GetMouseWorld(mouse_x, mouse_y);
	list<Unit*>::iterator i = enemy_units.begin();

	while (i != enemy_units.end())
	{
		if (mouse_x >= (*i)->sprite.position.x && mouse_x <= (*i)->sprite.position.x + (*i)->width && mouse_y >= (*i)->sprite.position.y && mouse_y <= (*i)->sprite.position.y + (*i)->height)
		{
			//Selected units attack target
			list<Unit*>::iterator sel_unit = selected_units.begin();
			while (sel_unit != selected_units.end())
			{
				if ((*sel_unit)->GetType() != MEDIC && (*sel_unit)->state != UNIT_DIE) //Medics doesn't attack
				{
					(*sel_unit)->avoid_change_state = false;
					App->tactical_ai->SetEvent(ENEMY_TARGET, (*sel_unit), (*i));
				}
				++sel_unit;
			}

			return;
		}
		++i;
	}

	//If we have ONLY 1 medic selected
	if (selected_units.size() == 1 && selected_units.front()->GetType() == MEDIC && selected_units.front()->state != UNIT_DIE)
	{
		list<Unit*>::iterator ally = friendly_units.begin();
		while (ally != friendly_units.end())
		{
			if (mouse_x >= (*ally)->sprite.position.x && mouse_x <= (*ally)->sprite.position.x + (*ally)->width && mouse_y >= (*ally)->sprite.position.y && mouse_y <= (*ally)->sprite.position.y + (*ally)->height)
			{
				selected_units.front()->avoid_change_state = false;
				App->tactical_ai->SetEvent(ENEMY_TARGET, selected_units.front(), (*ally));
				return;
			}
			++ally;
		}
	}

	//Nothing under
	SetMovement();
}

//CREATES -----------------------------------------------------------------------------------------------------

Unit* j1EntityManager::CreateUnit(UNIT_TYPE type, int x, int y, bool is_enemy)
{
	map<string, Unit*>::iterator it = units_database.find(UnitTypeToString(type));

	if (it != units_database.end())
	{
		Unit* unit = new Unit(it->second, is_enemy);
		unit->SetPosition(x, y);

		if (is_enemy)
			enemy_units.push_back(unit);
		else
			friendly_units.push_back(unit);

		//Creating health bar for the unit
		unit->hp_bar = App->ui->CreateBar("health", unit->max_life, unit->GetPosition().x, unit->GetPosition().y);
		unit->mana_bar = App->ui->CreateBar("progress", unit->max_mana, unit->GetPosition().x, unit->GetPosition().y);

		return unit;
	}
	else
		return NULL; //Unit type not found
}


void j1EntityManager::ActivateAbilities()
{
	//Only activate abilities with 1 unit selected - No multiple units abilities available
	if (selected_units.size() != 1)
		return;

	//Use abilities with keys for now - Implement with UI buttons
	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
	{
		(*selected_units.begin())->UseAbility(1);
	}

	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_UP)
	{
		(*selected_units.begin())->UseAbility(2);
	}
}

void j1EntityManager::DestroyBullet(Bullet* _bullet)
{
	list<Bullet*>::iterator i = bullets.begin();

	while (i != bullets.end())
	{
		if (*i == _bullet)
		{
			bullets.erase(i);
			delete _bullet;
			return;
		}
		++i;
	}

}

void j1EntityManager::RemoveBullet(Bullet* _bullet)
{
	if (_bullet != NULL)
		bullets_to_remove.push_back(_bullet);
}

void j1EntityManager::CleanUpList()
{
	list<Unit*>::iterator i = friendly_units.begin();

	while (i != friendly_units.end())
	{
		(*i)->CleanUp();

		delete (*i);
		(*i) = NULL;
		++i;
	}

	friendly_units.clear();

	list<Unit*>::iterator it = enemy_units.begin();

	while (it != enemy_units.end())
	{
		(*it)->CleanUp();

		delete (*it);
		(*it) = NULL;
		++it;
	}

	enemy_units.clear();
}