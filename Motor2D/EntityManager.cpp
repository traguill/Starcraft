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

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;
	//------------------------------------------------------------------------------

	//Basic logic
	SelectUnits();

	if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_UP)
	{
		CheckUnderCursor(); //Checks whats under the cursor position (enemy->attack, nothing->move)
	}

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
	
	if (units_to_remove.size() > 0)
	{
		//Check if the memory is ok
		_ASSERTE(_CrtCheckMemory());

		LOG("(Manager): Some units need to be destroyed");
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

	App->tex->UnLoad(gui_cursor);
	App->tex->UnLoad(health_bar);

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
			friendly_units.erase(f_unit);

			list<Unit*>::iterator s_unit = selected_units.begin();
			while (s_unit != selected_units.end())
			{
				if (*s_unit == _unit)
				{
					selected_units.erase(s_unit);
					RELEASE(_unit);
					return;
				}
				++s_unit;
			}
			RELEASE(_unit);
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
			RELEASE(_unit);
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

		unit_db->walk_anim_speed = unit.child("animwalkspeed").attribute("value").as_float();
		unit_db->idle_anim_speed = unit.child("animidlespeed").attribute("value").as_float();
		unit_db->attack_anim_speed = unit.child("animattackspeed").attribute("value").as_float();

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

				if (App->pathfinding->CreatePath(center_map, destination) == -1)

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
						if(App->pathfinding->CreatePath(unit_map_pos, end_point) != 1)
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
				App->tactical_ai->SetEvent(ENEMY_TARGET, (*sel_unit), (*i));
				++sel_unit;
			}

			return;
		}
		++i;
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
		//unit->hp_bar = App->ui->CreateBar(unit->life, unit->GetPosition().x, unit->GetPosition().y, 19, 5, SDL_Rect{ 0, 0, 19, 5 }, SDL_Rect{ 0, 15, 19, 5 }, SDL_Rect{ 0, 10, 19, 5 }, SDL_Rect{ 0, 5, 19, 5 }, health_bar);

		return unit;
	}
	else
		return NULL; //Unit type not found
}
