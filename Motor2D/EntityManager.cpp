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
#include "GameScene.h"


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

	//Debug
	jimmy = CreateUnit(MARINE, 200, 300);
	leroy_jenkins = CreateUnit(MARINE, 220, 300);

	friendly_units.push_back(jimmy);
	friendly_units.push_back(leroy_jenkins);

	return ret;
}

// Update all UIManagers
bool j1EntityManager::PreUpdate()
{
	return true;
}

bool j1EntityManager::Update(float dt)
{
	//Create units
	//DEBUG-----------------------------------------------------------------------------
	if (App->input->GetKey(SDL_SCANCODE_G) == KEY_DOWN)
	{
		iPoint p;  App->input->GetMouseWorld(p.x, p.y);
		friendly_units.push_back(CreateUnit(GHOST, p.x, p.y));
	}

	if (App->input->GetKey(SDL_SCANCODE_M) == KEY_DOWN)
	{
		iPoint p;  App->input->GetMouseWorld(p.x, p.y);
		friendly_units.push_back(CreateUnit(MARINE, p.x, p.y));
	}

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;

	if (debug)
	{
		//App->render->DrawQuad(move_rec, 255, 255, 0, 255, false, true); Doesn't follow the units when move
	}


	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_UP)
	{
		leroy_jenkins->state = UNIT_ATTACK;
		leroy_jenkins->target = jimmy;
	}
		
	//------------------------------------------------------------------------------

	//Basic logic
	SelectUnits();

	SetMovement();
	
	CheckCollisions();

	//Update lists
	list<Unit*>::iterator it = friendly_units.begin();
	while (it != friendly_units.end())
	{
		if (App->game_scene->GamePaused() == false)
			(*it)->Update(dt);
		
		(*it)->Draw();
		it++;
	}

	list<Unit*>::iterator i = enemy_units.begin();
	while (i != enemy_units.end())
	{
		if (App->game_scene->GamePaused() == false)
			(*i)->Update(dt);
		
		(*i)->Draw();
		i++;
	}

	//Sort 2 lists of elements

	return true;
}

// Called after all Updates
bool j1EntityManager::PostUpdate()
{
	list<Unit*>::iterator i = units_to_remove.begin();

	while (i != units_to_remove.end())
	{
		list<Unit*>::iterator unit_to_remove = i;
		++i;
		DestroyUnit((*unit_to_remove));
	}

	units_to_remove.clear();

	return true;
}

bool j1EntityManager::CleanUp()
{
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

	return true;
}

void j1EntityManager::RemoveUnit(Unit* _unit)
{
	if (_unit != NULL)
		units_to_remove.push_back(_unit);
}

void j1EntityManager::DestroyUnit(Unit* _unit)
{
	list<Unit*>::iterator f_unit = friendly_units.begin();

	while (f_unit != friendly_units.end())
	{
		if (*f_unit == _unit)
		{
			friendly_units.erase(f_unit);
			return;
		}
		++f_unit;
	}

	list<Unit*>::iterator e_unit = enemy_units.begin();

	while (e_unit != enemy_units.end())
	{
		if (*e_unit == _unit)
		{
			enemy_units.erase(e_unit);
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

	pugi::xml_node unit;
	for (unit = units.child("unit"); unit && ret; unit = unit.next_sibling("unit"))
	{
		Unit* unit_db = new Unit();
		unit_db->sprite.texture = App->tex->Load(unit.child("texture_path").attribute("value").as_string());
		unit_db->life = unit.child("life").attribute("value").as_int();
		unit_db->speed = unit.child("speed").attribute("value").as_int();
		unit_db->damage = unit.child("damage").attribute("value").as_int();
		unit_db->vision = unit.child("vision").attribute("value").as_int();
		unit_db->range = unit.child("range").attribute("value").as_int();
		unit_db->cool = unit.child("cool").attribute("value").as_int();
		unit_db->type = UnitTypeToEnum(unit.attribute("TYPE").as_string());
		unit_db->width = unit.child("width").attribute("value").as_int();
		unit_db->height = unit.child("height").attribute("value").as_int();

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

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		list<Unit*>::iterator it = selected_units.begin();
		while (it != selected_units.end())
		{
			(*it)->selected = false;

			it++;
		}

		selected_units.clear();
		App->input->GetMouseWorld(select_start.x, select_start.y);
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

				if (App->pathfinding->CreatePath(center_map, destination) == -1)
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

			

			list<Unit*>::iterator unit_p = selected_units.begin();
			while (unit_p != selected_units.end())
			{
				iPoint unit_pos = (*unit_p)->GetPosition();
				iPoint unit_pos_tile(App->map->WorldToMap(unit_pos.x, unit_pos.y, 2));

				iPoint dst_center(unit_pos_tile.x - center_map.x, unit_pos_tile.y - center_map.y);

				vector<iPoint> unit_path;

				vector<iPoint>::iterator path_it = path.begin();
				while (path_it != path.end())
				{
					unit_path.push_back(iPoint(path_it->x + dst_center.x, path_it->y + dst_center.y));
					++path_it;
				}

				(*unit_p)->SetPath(unit_path);
				++unit_p;
			}

		}
	}
}

void j1EntityManager::CalculateMovementRect()
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

void j1EntityManager::CheckCollisions()
{
	CheckCollisionsLists(friendly_units, friendly_units);
	CheckCollisionsLists(friendly_units, enemy_units);
	CheckCollisionsLists(enemy_units, enemy_units);
}

void j1EntityManager::CheckCollisionsLists(list<Unit*> list_a, list<Unit*> list_b)
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

			if ((*unit_a)->GetPosition().DistanceTo((*unit_b)->GetPosition()) <= COLLISION_DISTANCE)
			{
				if ((*unit_a)->state == UNIT_MOVE && (*unit_b)->state != UNIT_MOVE)
					SeparateUnits(*unit_a, *unit_b);

				if ((*unit_b)->state == UNIT_MOVE && (*unit_a)->state != UNIT_MOVE)
					SeparateUnits(*unit_b, *unit_a);
			}
			++count_b;
			++unit_b;
		}
		++count_a;
		++unit_a;
	}
}

void j1EntityManager::SeparateUnits(Unit* unit_a, Unit* unit_b)
{
	//Ignore collisions between TWO moving units
	//No one is moving (Can really happen?)

	iPoint direction = unit_a->GetDirection();
	iPoint move_to(0, 0);

	bool direction_found = false;
	int iterations = 0; //Max 8 iterations (directions)

	while (direction_found == false && iterations < 8)
	{
		//Move to direction + 1/4
		if (direction.x == 1 && direction.y == 0) move_to = { 2, 2 };
		if (direction.x == 1 && direction.y == 1) move_to = { 0, 2 };
		if (direction.x == 0 && direction.y == 1) move_to = { -2, 2 };
		if (direction.x == -1 && direction.y == 1) move_to = { -2, 0 };
		if (direction.x == -1 && direction.y == 0) move_to = { -2, -2 };
		if (direction.x == -1 && direction.y == -1) move_to = { 0, -2 };
		if (direction.x == 0 && direction.y == -1) move_to = { 2, -2 };
		if (direction.x == 1 && direction.y == -1) move_to = { 2, 0 };

		iPoint unit_tile = App->map->WorldToMap(unit_b->GetPosition().x, unit_b->GetPosition().y, COLLIDER_MAP);

		if (App->pathfinding->IsWalkable(unit_tile + move_to) == true)
		{
			//Create path for idle unit
			if (App->pathfinding->CreatePath(unit_tile, unit_tile + move_to) != -1)
			{
				unit_b->SetPath(*App->pathfinding->GetLastPath());
			}
			direction_found = true;
		}
		else
		{
			direction = move_to;
			++iterations;
		}
	}
	
}

//CREATES -----------------------------------------------------------------------------------------------------

Unit* j1EntityManager::CreateUnit(UNIT_TYPE type, int x, int y)
{
	map<string, Unit*>::iterator it = units_database.find(UnitTypeToString(type));

	if (it != units_database.end())
	{
		Unit* unit = new Unit(it->second);
		unit->SetPosition(x, y);

		return unit;
	}
	else
		return NULL; //Unit type not found
}