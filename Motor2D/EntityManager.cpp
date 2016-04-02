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
		
	//------------------------------------------------------------------------------

	//Check units selection
	SelectUnits();

	//Asign unit movement
	SetMovement();

	list<Unit*>::iterator it = friendly_units.begin();
	while (it != friendly_units.end())
	{
		if (App->game_scene->GamePaused() == false)
			(*it)->Update(dt);
		
		(*it)->Draw();
		it++;
	}

	list<Entity*>::iterator i = hostile_enities.begin();
	while (i != hostile_enities.end())
	{
		if (App->game_scene->GamePaused() == false)
			(*i)->Update(dt);
		
		(*i)->Draw();
		i++;
	}

	return true;
}

// Called after all Updates
bool j1EntityManager::PostUpdate()
{

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

	list<Entity*>::iterator it_en = hostile_enities.begin();
	while (it_en != hostile_enities.end())
	{
		delete *it_en;
		++it_en;
	}
	hostile_enities.clear();

	return true;
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
		unit_db->texture = App->tex->Load(unit.child("texture_path").attribute("value").as_string());
		unit_db->life = unit.child("life").attribute("value").as_int();
		unit_db->speed = unit.child("speed").attribute("value").as_int();
		unit_db->damage = unit.child("damage").attribute("value").as_int();
		unit_db->vision = unit.child("vision").attribute("value").as_int();
		unit_db->range = unit.child("range").attribute("value").as_int();
		unit_db->cool = unit.child("cool").attribute("value").as_int();
		unit_db->type = UnitTypeToEnum(unit.attribute("TYPE").as_string());

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
		App->input->GetMouseWorld(selection_rect.x, selection_rect.y);
	}

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP)
	{
		iPoint p; App->input->GetMouseWorld(p.x, p.y);
		selection_rect.w = p.x - selection_rect.x;
		selection_rect.h = p.y - selection_rect.y;

		list<Unit*>::iterator it = friendly_units.begin();

		while (it != friendly_units.end())
		{
			if ((*it)->pos.PointInRect(selection_rect.x, selection_rect.y, selection_rect.w, selection_rect.h) == true)
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
			iPoint center_map = App->map->WorldToMap(center.x, center.y, 2);

			App->pathfinding->CreatePath(center_map, destination);

			vector<iPoint> path = *App->pathfinding->GetLastPath();

			list<Unit*>::iterator unit_p = selected_units.begin();
			while (unit_p != selected_units.end())
			{
				iPoint unit_pos_tile(App->map->WorldToMap((*unit_p)->pos.x, (*unit_p)->pos.y, 2));
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
		//First time
		if (max_x == -1)
		{
			min_x = max_x = (*it)->pos.x;
			min_y = max_y = (*it)->pos.y;
		}

		if ((*it)->pos.x < min_x)  min_x = (*it)->pos.x;
		if ((*it)->pos.x > max_x)  max_x = (*it)->pos.x;
		if ((*it)->pos.y < min_y)  min_y = (*it)->pos.y;
		if ((*it)->pos.y > max_y)  max_y = (*it)->pos.y;

		++it;
	}

	move_rec.x = min_x;
	move_rec.y = min_y;
	move_rec.w = max_x - min_x;
	move_rec.h = max_y - min_y;

	center.x = move_rec.x + (move_rec.w / 2);
	center.y = move_rec.y + (move_rec.h / 2);
}

//CREATES -----------------------------------------------------------------------------------------------------

Unit* j1EntityManager::CreateUnit(UNIT_TYPE type, int x, int y)
{
	map<string, Unit*>::iterator it = units_database.find(UnitTypeToString(type));

	if (it != units_database.end())
	{
		Unit* unit = new Unit(*it->second);
		unit->pos.x = x;
		unit->pos.y = y;

		return unit;
	}
	else
		return NULL; //Unit type not found
}