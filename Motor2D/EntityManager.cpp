#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Fonts.h"
#include "j1Input.h"
#include "EntityManager.h"
#include "UILabel.h"
#include "UIImage.h"
#include "UIButton.h"
#include "j1Window.h"
#include "UIInputBox.h"
#include "j1FileSystem.h"


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

	return ret;
}

// Update all UIManagers
bool j1EntityManager::PreUpdate()
{
	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		iPoint p;  App->input->GetMousePosition(p.x, p.y);
		friendly_units.insert(pair<string, Unit*>("Jimmy", CreateUnit(GHOST, p.x, p.y)));
	}

	if (App->input->GetMouseButtonDown(SDL_BUTTON_MIDDLE) == KEY_DOWN)
	{
		iPoint p;  App->input->GetMousePosition(p.x, p.y);
		friendly_units.insert(pair<string, Unit*>("Thom", CreateUnit(MARINE, p.x, p.y)));
	}
	return true;
}

bool j1EntityManager::Update(float dt)
{
	map<string, Unit*>::iterator it = friendly_units.begin();
	while (it != friendly_units.end())
	{
		(*it).second->Update(dt);
		(*it).second->Draw();
		it++;
	}

	list<Entity*>::iterator i = hostile_enities.begin();
	while (i != hostile_enities.end())
	{
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

	map<string, Unit*>::iterator it_fu = friendly_units.begin();
	while (it_fu != friendly_units.end())
	{
		delete it_fu->second;
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