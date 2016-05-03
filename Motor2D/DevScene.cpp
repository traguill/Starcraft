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

	//Load Map
	App->map->Load("0.5_game_map.tmx", map_id);

	//Load collision map
	if (App->map->Load("collision.tmx", collider_id) == true)
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


	LoadLevel();

	App->render->camera = SDL_Rect{ -700, -150, App->render->camera.w, App->render->camera.h };

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
	App->map->Draw(map_id);





	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;

	if (debug)
		App->map->Draw(collider_id);

	if (App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN)
		game_paused = !game_paused;

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


	return true;
}


bool DevScene::GamePaused()const
{
	return game_paused;
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

	pugi::xml_node unit_f;
	for (unit_f = level.child("friendly_unit"); unit_f; unit_f = unit_f.next_sibling("friendly_unit"))
	{
		UNIT_TYPE type = App->entity->UnitTypeToEnum(unit_f.child("type").attribute("value").as_string());
		iPoint pos;
		pos.x = unit_f.child("position").attribute("x").as_int();
		pos.y = unit_f.child("position").next_sibling("position").attribute("y").as_int();
		bool is_enemy = unit_f.child("is_enemy").attribute("value").as_bool();
		App->entity->CreateUnit(type, pos.x, pos.y, is_enemy);
	}

	pugi::xml_node unit_e;
	for (unit_e = level.child("enemy_unit"); unit_e; unit_e = unit_e.next_sibling("enemy_unit"))
	{
		UNIT_TYPE type = App->entity->UnitTypeToEnum(unit_e.child("type").attribute("value").as_string());
		iPoint pos;
		pos.x = unit_e.child("position").attribute("x").as_int();
		pos.y = unit_e.child("position").next_sibling("position").attribute("y").as_int();
		bool is_enemy = unit_e.child("is_enemy").attribute("value").as_bool();
		App->entity->CreateUnit(type, pos.x, pos.y, is_enemy);
	}
}

void DevScene::SaveLevelDesign()
{
	// xml object were we will store all data
	pugi::xml_document data;
	pugi::xml_node root;

	root = data.append_child("level");

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





