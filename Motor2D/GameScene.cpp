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
#include "GameScene.h"
#include "j1UIManager.h"
#include "j1FileSystem.h"
#include "EventsManager.h"
#include "EntityManager.h"
#include "SceneManager.h"

GameScene::GameScene() : j1Module()
{
	name.append("scene");
}

// Destructor
GameScene::~GameScene()
{}

// Called before render is available
bool GameScene::Awake(pugi::xml_node& conf)
{
	LOG("Loading Scene");
	bool ret = true;

	return ret;
}

// Called before the first frame
bool GameScene::Start()
{

	//Setting fx volume
	App->audio->SetFxVolume(30);

	//Load Map
	App->map->Load("0.5_game_map.tmx", map_id);

	//Load collision map
	if (App->map->Load("collision.tmx", collider_id) == true)
	{
		int width, height;
		uchar* buffer = NULL;
		App->map->CreateWalkabilityMap(width, height, &buffer, collider_id);

		App->pathfinding->SetMap(width, height, buffer);
		RELEASE_ARRAY(buffer);


		//Create HUD
		App->ui->CreateImage({ 0, 396, 637, 192 }, 2, 290, true);


		//MARINE
		SDL_Rect s_armourM{ 621, 62, 34, 33 };
		marine_armour_icon = App->ui->CreateImage(s_armourM, 240, 440, false);


		SDL_Rect s_weaponM{ 507, 62, 34, 33 };
		marine_weapon_icon = App->ui->CreateImage(s_weaponM, 275, 440, false);


		SDL_Rect s_wireframeM{ 705, 126, 54, 70 };
		marine_wireframe = App->ui->CreateImage(s_wireframeM, 180, 390, false);


		//GHOST
		SDL_Rect s_weaponG{ 543, 62, 34, 33 };
		ghost_weapon_icon = App->ui->CreateImage(s_weaponG, 275, 440, false);


		SDL_Rect s_wireframeG{ 706, 206, 54, 71 };
		ghost_wireframe = App->ui->CreateImage(s_wireframeG, 180, 390, false);


		//FIREBAT
		SDL_Rect s_weaponF{ 582, 62, 34, 33 };
		firebat_weapon_icon = App->ui->CreateImage(s_weaponF, 275, 440, false);


		SDL_Rect s_wireframeF{ 791, 128, 54, 70 };
		firebat_wireframe = App->ui->CreateImage(s_wireframeF, 180, 390, false);


		//MEDIC
		SDL_Rect s_wireframeMed{ 797, 208, 47, 66 };
		medic_wireframe = App->ui->CreateImage(s_wireframeMed, 180, 390, false);


		//OBSERVER
		SDL_Rect s_wireframeO{ 621, 138, 54, 55 };
		observer_wireframe = App->ui->CreateImage(s_wireframeO, 180, 390, false);

		//CREATE EVENT
		objectives_box = App->ui->CreateImage(SDL_Rect{ 0, 90, 169, 71 }, 470, -5, true);
		objective_info_1 = App->ui->CreateLabel("You must retrieve the bomb ", 477, 5);
		objective_info_2 = App->ui->CreateLabel("from the southern enemy", 477, 15);
		objective_info_3 = App->ui->CreateLabel("base", 477, 25);
		pause_mark = App->ui->CreateImage(SDL_Rect{66, 162, 56, 38}, 470 - 56, 0, false);
		run_mark = App->ui->CreateImage(SDL_Rect{ 0, 162, 56, 38 }, 470 - 56, 0, true);

		//Pathfinding Label
		pathfinding_label = App->ui->CreateLabel("I can't go there Sir!", 100, 50, true);
		pathfinding_label->is_visible = false;

		App->events->game_event = INITIAL_STATE;

		//Life in HUD
		life_HUD = App->ui->CreateLabel("", 177, 458);
		
		//Sniper
		snipper_ui = App->ui->CreateImage({ 0, 651, 640, 480 }, 0, 0, false);		
	}

	//Win image & button
	win_background = App->ui->CreateImage({ 847, 1, 221, 108 }, 220, 150, false, true);
	win_button = App->ui->CreateButton("", 304, 246, { 847, 137, 53, 23 }, { 847, 163, 53, 23 }, { 847, 111, 53, 23 }, this);
	//win_button->SetParent(win_background);
	win_button->SetVisible(false);

	//Loose image & button
	loose_background = App->ui->CreateImage({ 1073, 1, 221, 108 }, 220, 150, false, true);
	loose_button = App->ui->CreateButton("", 304, 246, { 902, 137, 53, 23 }, { 902, 163, 53, 23 }, { 902, 111, 53, 23 }, this);
	//loose_button->SetParent(loose_background);
	loose_button->SetVisible(false);

	//Bomb
	bomb = App->tex->Load("sprites/Bomb.png");
	bomb_rect = { 30, 12, 32, 32 };
	bomb_zone = { 815, 3080, 50, 50 };
	bomb_pos.x = 926;
	bomb_pos.y = 1403;

	bomb_available = false;

	debug = false;
	game_paused = false;

	//UI TESTS
	//
	//Not able to use labels, missing font;
	//----------App->ui->CreateLabel("hola", 100, 100);
	//----------App->ui->CreateButton("hola", 100, 100, { 0,0,50,50 }, { 0, 0, 0, 500 }, { 0, 0, 0, 0 });
	
	music = App->audio->PlayMusic("StarcraftTerrantheme1.wav");

	LoadLevel();

	App->render->camera = SDL_Rect{ -700, -150, App->render->camera.w, App->render->camera.h };

	return true;
}

// Called each loop iteration
bool GameScene::PreUpdate()
{

	return true;
}

// Called each loop iteration
bool GameScene::Update(float dt)
{
	App->map->Draw(map_id);

	if (App->entity->debug)
	{
		App->render->DrawQuad({ bomb_pos.x, bomb_pos.y, bomb_rect.w, bomb_rect.h }, 255, 255, 0, 255, true, true);
	}

	//Updating timer
	if (parthfinding_label_timer.ReadSec() >= 3)
	{
		pathfinding_label->is_visible = false;
	}

	if (bomb_available == false)
		App->render->Blit(bomb, bomb_pos.x, bomb_pos.y, &bomb_rect);
	else
		App->render->DrawQuad(bomb_zone, 255, 255, 0, 125, true, true);

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;
	if (debug)
		App->map->Draw(collider_id);

	if (App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN)
		game_paused = !game_paused;


	//Save level designed
	if (App->input->GetKey(SDL_SCANCODE_KP_8) == KEY_UP)
	{
		SaveLevelDesign();
	}


	if (GamePaused())
	{
		run_mark->is_visible = false;
		pause_mark->is_visible = true;
	}

	else
	{
		run_mark->is_visible = true;
		pause_mark->is_visible = false;
	}


	//Check win or lose
	if (App->entity->friendly_units.size() == 0)
	{
		loose_background->is_visible = true;
		loose_button->is_visible = true;
		game_paused = true;
	}

	if (bomb_available == false)
	{
		list<Unit*>::iterator f_unit = App->entity->friendly_units.begin();
		while (f_unit != App->entity->friendly_units.end())
		{
			iPoint pos = (*f_unit)->GetPosition();

			if (pos.x > bomb_pos.x && pos.x < bomb_pos.x + bomb_rect.w && pos.y > bomb_pos.y && pos.y < bomb_pos.y + bomb_rect.w && (*f_unit)->IsVisible())
			{
				LOG("YOU HAVE THE BOMB");
				App->events->game_event = BOMB_RETRIVED;
				bomb_available = true;
			}

			++f_unit;
		}
	}
	else
	{
		list<Unit*>::iterator f_unit = App->entity->friendly_units.begin();
		while (f_unit != App->entity->friendly_units.end())
		{
			iPoint pos = (*f_unit)->GetPosition();

			if (pos.x > bomb_zone.x && pos.x < bomb_zone.x + bomb_zone.w && pos.y > bomb_zone.y && pos.y < bomb_zone.y + bomb_zone.w && (*f_unit)->IsVisible())
			{
				win_background->is_visible = true;
				win_button->is_visible = true;
				game_paused = true;
			}

			++f_unit;
		}
	}
	


	return true;
}

// Called each loop iteration
bool GameScene::PostUpdate()
{
	bool ret = true;



	return ret;
}

// Called before quitting
bool GameScene::CleanUp()
{
	LOG("Freeing Game Scene");



	return true;
}


bool GameScene::GamePaused()const
{
	return game_paused;
}

void GameScene::LoadLevel()
{
	pugi::xml_document	level_file;
	pugi::xml_node		level;

	char* buf;
	int size = App->fs->Load("my_level.xml", &buf);
	pugi::xml_parse_result result = level_file.load_buffer(buf, size);
	RELEASE(buf);

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

void GameScene::SaveLevelDesign()
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

void GameScene::OnGUI(UIEntity* gui, GUI_EVENTS event)
{
	if (gui->type == BUTTON)
	{
		if ((UIButton*)gui == win_button && event == MOUSE_BUTTON_RIGHT_UP)
		{
			App->scene_manager->WantToChangeScene();
		}

		else if ((UIButton*)gui == loose_button && event == MOUSE_BUTTON_RIGHT_UP)
		{
			App->scene_manager->WantToChangeScene();
		}
	}
}