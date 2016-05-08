#include <sstream> 
#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1Map.h"
#include "j1PathFinding.h"
#include "DevScene.h"
#include "j1UIManager.h"
#include "j1FileSystem.h"
#include "EventsManager.h"
#include "EntityManager.h"
#include "SceneManager.h"

DevScene::DevScene() : j1Module()
{
	name.append("dev_scene");
}

// Destructor
DevScene::~DevScene()
{}

// Called before render is available
bool DevScene::Awake(pugi::xml_node& conf)
{
	LOG("Loading Developing Scene");
	bool ret = true;

	return ret;
}

// Called before the first frame
bool DevScene::Start()
{
	selected_units.clear();

	//Load Map
	App->map->Load("Jungle Map 32.tmx", map_id);

	//Load collision map
	if (App->map->Load("Collision Jungle.tmx", collider_id) == true)
	{
		int width, height;
		uchar* buffer = NULL;
		App->map->CreateWalkabilityMap(width, height, &buffer, collider_id);

		App->pathfinding->SetMap(width, height, buffer);
		delete[] buffer;
		buffer = NULL;
	}

	debug = false;
	game_paused = false;

	App->render->camera = SDL_Rect{ -700, -150, App->render->camera.w, App->render->camera.h };

	bomb = App->tex->Load("sprites/Bomb.png");
	bomb_rect = { 30, 12, 32, 32 };
	bomb_zone = { 700, 150, 50, 50 };
	bomb_pos.x = 900;
	bomb_pos.y = 150;

	LoadLevel();

	return true;
}

// Called each loop iteration
bool DevScene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool DevScene::Update(float dt)
{
	//Logic
	UnitCreation();

	UnitMovement();

	DeleteUnits(); //Check supr key

	AsignPatrol();

	SetDirection();

	BombMovement();

	App->render->CursorMovement(dt);

	//Draw
	App->map->Draw(map_id);

	//DrawSelection
	DrawSelection();

	//Draw Bomb & Zone
	App->render->Blit(bomb, bomb_pos.x, bomb_pos.y, &bomb_rect);
	
	App->render->DrawQuad(bomb_zone, 255, 255, 0, 125, true, true);

	//Save and load
	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_UP)
		SaveLevelDesign();

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_UP)
		debug = !debug;

	if (debug)
		App->map->Draw(COLLIDER_MAP);

	return true;
}

// Called each loop iteration
bool DevScene::PostUpdate()
{
	bool ret = true;

	return ret;
}

// Called before quitting
bool DevScene::CleanUp()
{
	LOG("Freeing Game Scene");

	App->tex->UnLoad(bomb);
	bomb = NULL;
	selected_units.clear();

	return true;
}


bool DevScene::GamePaused()const
{
	return game_paused;
}

void DevScene::BombMovement()
{
	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT)
	{
		iPoint mouse;
		App->input->GetMouseWorld(mouse.x, mouse.y);
		//Bomb
		if (PointInRect(mouse, { bomb_pos.x, bomb_pos.y, bomb_rect.w, bomb_rect.h }))
		{
			iPoint motion;
			App->input->GetMouseMotion(motion.x, motion.y);
			bomb_pos.x += motion.x;
			bomb_pos.y += motion.y;
		}

		//End zone
		if (PointInRect(mouse, bomb_zone))
		{
			iPoint motion;
			App->input->GetMouseMotion(motion.x, motion.y);
			bomb_zone.x += motion.x;
			bomb_zone.y += motion.y;
		}
		
	}
}

void DevScene::SetDirection()
{
	if (selected_units.size() == 1)
	{
		if (App->input->GetMouseButtonDown(SDL_BUTTON_MIDDLE) == KEY_UP)
		{
			Unit* unit = selected_units.front();
			iPoint mouse;
			App->input->GetMouseWorld(mouse.x, mouse.y);
			unit->direction.x = mouse.x - unit->GetPosition().x;
			unit->direction.y = mouse.y - unit->GetPosition().y;
		}
	}
}

void DevScene::UnitCreation()
{
	iPoint mouse;
	App->input->GetMouseWorld(mouse.x, mouse.y);
	vector<iPoint> patrol; //Patrolling stuff (useless)
	if (App->input->GetKey(SDL_SCANCODE_1) == KEY_UP) //MARINE
	{
		App->entity->CreateUnit(MARINE, mouse.x, mouse.y, false, false, patrol);
	}
	if (App->input->GetKey(SDL_SCANCODE_2) == KEY_UP) //FIREBAT
	{
		App->entity->CreateUnit(FIREBAT, mouse.x, mouse.y, false, false, patrol);
	}
	if (App->input->GetKey(SDL_SCANCODE_3) == KEY_UP) //MEDIC
	{
		App->entity->CreateUnit(MEDIC, mouse.x, mouse.y, false, false, patrol);
	}
	if (App->input->GetKey(SDL_SCANCODE_4) == KEY_UP) //GHOST
	{
		App->entity->CreateUnit(GHOST, mouse.x, mouse.y, false, false, patrol);
	}
	if (App->input->GetKey(SDL_SCANCODE_5) == KEY_UP) //MARINE ENEMY
	{
		App->entity->CreateUnit(MARINE, mouse.x, mouse.y, true, false, patrol);
	}
	if (App->input->GetKey(SDL_SCANCODE_6) == KEY_UP) //FIREBAT ENEMY
	{
		App->entity->CreateUnit(FIREBAT, mouse.x, mouse.y, true, false, patrol);
	}
}

void DevScene::AsignPatrol()
{
	if (selected_units.size() == 1)
	{
		
		//Assign patrol
		Unit* unit = selected_units.front();
		if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_UP)
		{
			unit->patrol = true;
			iPoint mouse;
			App->input->GetMouseWorld(mouse.x, mouse.y);
			mouse = App->map->WorldToMap(mouse.x, mouse.y, COLLIDER_MAP);
			unit->patrol_path.push_back(mouse);
		}
		else
		{
			if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_UP)
			{
				if (unit->patrol_path.size() != 0)
				{
					unit->patrol_path.pop_back();
					if (unit->patrol_path.size() == 0)
						unit->patrol = false;
				}
			}
		}
	}
	list<Unit*>::iterator unit = selected_units.begin();
	while (unit != selected_units.end())
	{
		//Start or Stop patrol
		if (App->input->GetKey(SDL_SCANCODE_KP_ENTER) == KEY_UP)
		{
			//Start
			if ((*unit)->patrol == false && (*unit)->patrol_path.size() != 0)
			{
				(*unit)->patrol = true;
			}
			else
			{
				//Stop
				if ((*unit)->patrol == true && (*unit)->patrol_path.size() != 0)
				{
					(*unit)->patrol = false;
					iPoint origin = App->map->MapToWorld((*unit)->patrol_path.front().x, (*unit)->patrol_path.front().y, COLLIDER_MAP);
					(*unit)->SetPosition(origin.x, origin.y);
					(*unit)->state = UNIT_IDLE;
				}
			}

		}
		++unit;
	}
	
}

void DevScene::DeleteUnits()
{
	if (App->input->GetKey(SDL_SCANCODE_DELETE) == KEY_UP)
	{
		list<Unit*>::iterator it = selected_units.begin();

		while (it != selected_units.end())
		{
			App->entity->RemoveUnit((*it));
			++it;
		}
		selected_units.clear();
	}
}

void DevScene::UnitMovement()
{
	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		iPoint mouse;
		App->input->GetMouseWorld(mouse.x, mouse.y);

		list<Unit*>::iterator f_unit = App->entity->friendly_units.begin();
		while (f_unit != App->entity->friendly_units.end())
		{
			if (PointInRect(mouse, { (*f_unit)->GetDrawPosition().x, (*f_unit)->GetDrawPosition().y, (*f_unit)->width, (*f_unit)->height }) == true)
			{
				if(Find(*f_unit) == false)
					selected_units.push_back(*f_unit);
				return;
			}
			++f_unit;
		}

		list<Unit*>::iterator e_unit = App->entity->enemy_units.begin();
		while (e_unit != App->entity->enemy_units.end())
		{
			if (PointInRect(mouse, { (*e_unit)->GetDrawPosition().x, (*e_unit)->GetDrawPosition().y, (*e_unit)->width, (*e_unit)->height }) == true)
			{
				if (Find(*e_unit) == false)
					selected_units.push_back(*e_unit);
				return;
			}
			++e_unit;
		}

		selected_units.clear();
	}


	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT)
	{
		list<Unit*>::iterator it = selected_units.begin();

		while (it != selected_units.end())
		{
			iPoint motion;
			App->input->GetMouseMotion(motion.x, motion.y);
			(*it)->SetPosition((*it)->GetPosition().x + motion.x, (*it)->GetPosition().y + motion.y);

			++it;
		}	
	}

}

bool DevScene::Find(Unit* u)
{
	list<Unit*>::iterator it = selected_units.begin();
	while (it != selected_units.end())
	{
		if (*it == u)
			return true;
		++it;
	}

	return false;
}

void DevScene::DrawSelection()
{
	list<Unit*>::iterator it = selected_units.begin();
	while (it != selected_units.end())
	{
		iPoint up_left = (*it)->GetDrawPosition();
		App->render->DrawQuad({ up_left.x, up_left.y, (*it)->width, (*it)->height }, 0, 255, 255, 255, false, true);

		if ((*it)->patrol_path.size() != 0)
		{
			vector<iPoint>::iterator point = (*it)->patrol_path.begin();
			iPoint p0(-1, -1);
			while (point != (*it)->patrol_path.end())
			{
				iPoint p = App->map->MapToWorld(point->x, point->y, COLLIDER_MAP);
				App->render->DrawQuad({ p.x, p.y, 8, 8 }, 255, 0, 0, 255, true, true);

				if (p0.x != -1)
				{
					App->render->DrawLine(p.x, p.y, p0.x, p0.y, 0, 255, 0, 255, true);
				}
				p0 = p;
				++point;
			}
		}

		++it;
	}
}

bool DevScene::PointInRect(iPoint p, SDL_Rect rec)
{
	if (p.x >= rec.x && p.x <= rec.x + rec.w && p.y >= rec.y && p.y <= rec.y + rec.h)
		return true;
	else
		return false;
}

void DevScene::LoadLevel()
{
	pugi::xml_document	level_file;
	pugi::xml_node		level;

	char* buf;
	int size = App->fs->Load("my_level.xml", &buf);
	pugi::xml_parse_result result = level_file.load_buffer(buf, size);
	delete[] buf;
	buf = NULL;

	if (result == NULL)
	{
		LOG("Could not load xml file %s. PUGI error: &s", "my_level.xml", result.description());
		return;
	}
	else
		level = level_file.child("level");

	bomb_pos.x = level.child("bomb").attribute("x").as_int();
	bomb_pos.y = level.child("bomb").attribute("y").as_int();
	bomb_zone.x = level.child("bomb_zone").attribute("x").as_int();
	bomb_zone.y = level.child("bomb_zone").attribute("y").as_int();

	pugi::xml_node unit_f;
	for (unit_f = level.child("friendly_unit"); unit_f; unit_f = unit_f.next_sibling("friendly_unit"))
	{
		UNIT_TYPE type = App->entity->UnitTypeToEnum(unit_f.child("type").attribute("value").as_string());
		iPoint pos;
		pos.x = unit_f.child("position").attribute("x").as_int();
		pos.y = unit_f.child("position").next_sibling("position").attribute("y").as_int();
		bool is_enemy = unit_f.child("is_enemy").attribute("value").as_bool();
		bool patrolling = unit_f.child("patrol").attribute("value").as_bool();
		vector<iPoint> point_path;
		for (pugi::xml_node point = unit_f.child("patrol").child("point"); point; point = point.next_sibling("point"))
		{
			point_path.push_back({ point.attribute("tile_x").as_int(), point.attribute("tile_y").as_int() });
		}
		App->entity->CreateUnit(type, pos.x, pos.y, is_enemy, patrolling, point_path);
	}

	pugi::xml_node unit_e;
	for (unit_e = level.child("enemy_unit"); unit_e; unit_e = unit_e.next_sibling("enemy_unit"))
	{
		UNIT_TYPE type = App->entity->UnitTypeToEnum(unit_e.child("type").attribute("value").as_string());
		iPoint pos;
		pos.x = unit_e.child("position").attribute("x").as_int();
		pos.y = unit_e.child("position").next_sibling("position").attribute("y").as_int();
		bool is_enemy = unit_e.child("is_enemy").attribute("value").as_bool();
		bool patrolling = unit_e.child("patrol").attribute("value").as_bool();
		vector<iPoint> point_path;
		for (pugi::xml_node point = unit_e.child("patrol").child("point"); point; point = point.next_sibling("point"))
		{
			point_path.push_back({ point.attribute("tile_x").as_int(), point.attribute("tile_y").as_int() });
		}
		App->entity->CreateUnit(type, pos.x, pos.y, is_enemy, patrolling, point_path);
	}
}

void DevScene::SaveLevelDesign()
{
	// xml object were we will store all data
	pugi::xml_document data;
	pugi::xml_node root;

	root = data.append_child("level");

	root.append_child("bomb").append_attribute("x") = bomb_pos.x;
	root.child("bomb").append_attribute("y") = bomb_pos.y;
	
	root.append_child("bomb_zone").append_attribute("x") = bomb_zone.x;
	root.child("bomb_zone").append_attribute("y") = bomb_zone.y;

	list<Unit*>::iterator unit_f = App->entity->friendly_units.begin();
	while (unit_f != App->entity->friendly_units.end())
	{
		pugi::xml_node friend_unit;
		friend_unit = root.append_child("friendly_unit");

		friend_unit.append_child("type").append_attribute("value") = App->entity->UnitTypeToString((*unit_f)->GetType()).c_str();
		friend_unit.append_child("is_enemy").append_attribute("value") = (*unit_f)->is_enemy;
		friend_unit.append_child("position").append_attribute("x") = (*unit_f)->GetPosition().x;
		friend_unit.append_child("position").append_attribute("y") = (*unit_f)->GetPosition().y;

		friend_unit.append_child("direction").append_attribute("x") = (*unit_f)->direction.x;
		friend_unit.append_child("direction").append_attribute("y") = (*unit_f)->direction.y;

		friend_unit.append_child("patrol").append_attribute("value") = false; //Friendly Units dont patrol

		++unit_f;
	}

	list<Unit*>::iterator unit_e = App->entity->enemy_units.begin();
	while (unit_e != App->entity->enemy_units.end())
	{
		pugi::xml_node enemy_unit;
		enemy_unit = root.append_child("enemy_unit");

		enemy_unit.append_child("type").append_attribute("value") = App->entity->UnitTypeToString((*unit_e)->GetType()).c_str();
		enemy_unit.append_child("is_enemy").append_attribute("value") = (*unit_e)->is_enemy;
		enemy_unit.append_child("position").append_attribute("x") = (*unit_e)->GetPosition().x;
		enemy_unit.append_child("position").append_attribute("y") = (*unit_e)->GetPosition().y;

		enemy_unit.append_child("direction").append_attribute("x") = (*unit_e)->direction.x;
		enemy_unit.append_child("direction").append_attribute("y") = (*unit_e)->direction.y;
		
		//Make sure patrol is activated if patrol path exists
		if ((*unit_e)->patrol_path.size() > 0)
			(*unit_e)->patrol = true;

		enemy_unit.append_child("patrol").append_attribute("value") = (*unit_e)->patrol;

		pugi::xml_node patrol = enemy_unit.child("patrol");
		pugi::xml_node point(NULL);

		if ((*unit_e)->patrol_path.size() > 0)
		{
			vector<iPoint>::iterator tile = (*unit_e)->patrol_path.begin();
			while (tile != (*unit_e)->patrol_path.end())
			{
				
				enemy_unit.child("patrol").append_child("point").append_attribute("tile_x") = tile->x;

				if (point == NULL)
					point = patrol.child("point");
				else
					point = point.next_sibling("point");
			
				point.append_attribute("tile_y") = tile->y;
				++tile;
			}
		}

		++unit_e;
	}

	std::stringstream stream;
	data.save(stream);

	// we are done, so write data to disk
	App->fs->Save("my_level.xml", stream.str().c_str(), stream.str().length());

}

void DevScene::OnGUI(UIEntity* gui, GUI_EVENTS event)
{
	
}




