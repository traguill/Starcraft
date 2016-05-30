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
#include "InputManager.h"

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
	LOG("Starting GameScene");

	LoadAudio();

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

	LoadHUD();

	//Bomb
	bomb = App->tex->Load("sprites/Bomb.png");
	bzone = App->tex->Load("sprites/extraction.png");
	bomb_rect = { 30, 12, 32, 32 };
	bomb_zone = { 815, 1780, 164, 82 };
	

	debug = false;
	game_paused = false;

	if (App->scene_manager->level_saved == false)
	{
		sniper_ammo = 3;
		intel_left = 3;
		LoadGame("my_level.xml");
		App->render->camera = SDL_Rect{ -700, -1600, App->render->camera.w, App->render->camera.h };
	}
	else
		LoadGame("game_saved.xml");

	LoadAudio();

	App->audio->PlayMusic("StarcraftTerrantheme1.wav");

	game_finished = false;

	LoadQuitUI();

	if (App->scene_manager->level_saved != true)
		LoadTutorial();

	else
		tutorial_finished = true;

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
	if (tutorial_finished == false)
		game_paused = true;

	//Debug (go to menu)
	if (App->input->GetKey(SDL_SCANCODE_KP_1) == KEY_UP)
		App->scene_manager->WantToChangeScene(MENU);

	App->map->Draw(map_id);

	//Updating timer
	if (parthfinding_label_timer.ReadSec() >= 3)
	{
		pathfinding_label->is_visible = false;
	}

	if (game_saved_timer.ReadSec() >= 2)
	{
		game_saved->is_visible = false;
	}

	if (sniper_ui_timer.ReadSec() >= 3)
	{
		no_energy->is_visible = false;
		no_ammo->is_visible = false;
	}

	if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN && tutorial_finished)
	{
		game_paused = true;

		quit_fadeblack->SetVisible(true);
		App->ui->AnimResize(quit_button, 0.5f, true, 0.0f);
		App->ui->AnimResize(resume_button, 0.5f, true, 0.0f);
		quit_button->SetVisible(true);
		resume_button->SetVisible(true);
	}

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;
	if (debug)
		App->map->Draw(collider_id);
	
	//Check shortcuts
	list<ShortCut*>::iterator it = App->input_manager->shortcuts_list.begin();
	while (it != App->input_manager->shortcuts_list.end())
	{
		if ((*it)->name == "Pause" && (*it)->active)
		{
			game_paused = !game_paused;
			if (game_paused)
				App->ui->AnimResize(pause_mark, 0.1f, true);
			else
				App->ui->AnimResize(run_mark, 0.1f, true);
		}

			it++;
	}

	//Save level designed
	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_UP)
	{
		SaveGame("game_saved.xml");
		game_saved_timer.Start();
		game_saved->is_visible = true;
	}

	else if (App->input->GetKey(SDL_SCANCODE_L) == KEY_UP)
	{
		char* buf;
		int size = App->fs->Load("game_saved.xml", &buf);
		if (size > 0)
		{
			App->entity->CleanUpList();
			bomb_pos.clear();
			LoadGame("game_saved.xml");

			delete[] buf;
			buf = NULL;
		}
	}

	//Check win or lose
	if (App->entity->friendly_units.size() == 0)
	{
		LoseGameKilled();
	}

	if (bomb_pos.size() > 0)
	{
		list<Unit*>::iterator f_unit = App->entity->friendly_units.begin();
		while (f_unit != App->entity->friendly_units.end())
		{
			iPoint pos = (*f_unit)->GetPosition();

			list<iPoint>::iterator bomb_position = bomb_pos.begin();
			while (bomb_position != bomb_pos.end())
			{
				if (pos.x > (*bomb_position).x && pos.x < (*bomb_position).x + bomb_rect.w && pos.y > (*bomb_position).y && pos.y < (*bomb_position).y + bomb_rect.w)
				{
					App->game_scene->intel_left--;
					App->game_scene->sniper_ammo++;

					char ui_sniper_ammo[20];
					sprintf_s(ui_sniper_ammo, sizeof(ui_sniper_ammo), "Cal. 50 bullets: %d", sniper_ammo);
					sniper_ammo_label->Print(ui_sniper_ammo, false);

					if (intel_left > 0)
					{
						char ui_intel_left[20];
						sprintf_s(ui_intel_left, sizeof(ui_intel_left), "Bombs left: %d", intel_left);
						grenade_ammo_label->Print(ui_intel_left, false);
					}

					else
					{
						grenade_ammo_label->Print("Go to extraction point", false);
					}

					bomb_position = bomb_pos.erase(bomb_position);
				}
				++bomb_position;
			}
			

			++f_unit;
		}
	}
	else if(game_finished == false && ExtractionPoint() == true)
	{
		win_background->is_visible = true;
		win_button->is_visible = true;
		game_paused = true;
		game_finished = true;

		App->ui->AnimResize(win_background, 0.5f, true);
		App->ui->AnimResize(win_button, 0.25f, true, 0.3f);
	}

	if (bomb_pos.size() > 0)
	{
		list<iPoint>::iterator bomb_position = bomb_pos.begin();
		while (bomb_position != bomb_pos.end())
		{
			App->render->Blit(bomb, (*bomb_position).x, (*bomb_position).y, &bomb_rect);
			++bomb_position;
			
		}
	}
	else
	{
		SDL_Rect rec = { 0, 0, bomb_zone.w, bomb_zone.h };
		App->render->Blit(bzone, bomb_zone.x, bomb_zone.y, &rec);
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

	if (IsGhostSelected())
	{
		ghost_invisibility_button->SetVisible(true);
		ghost_snipermode_button->SetVisible(true);
	}
	else
	{
		ghost_invisibility_button->SetVisible(false);
		ghost_snipermode_button->SetVisible(false);
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

	App->map->UnLoad(map_id);
	App->map->UnLoad(collider_id);

	//life_HUD = NULL;
	minimap = NULL;

	marine_weapon_icon = NULL;
	marine_armour_icon = NULL;
	marine_wireframe = NULL;

	ghost_weapon_icon = NULL;
	ghost_wireframe = NULL;
	ghost_invisibility_button = NULL;
	ghost_snipermode_button = NULL;

	firebat_weapon_icon = NULL;
	firebat_wireframe = NULL;

	medic_wireframe = NULL;

	observer_wireframe = NULL;

	snipper_ui = NULL;

	objective_info_1 = NULL;
	objective_info_2 = NULL;
	objective_info_3 = NULL;
	objectives_box = NULL;
	pause_mark = NULL;
	run_mark = NULL;

	pathfinding_label = NULL;

	no_ammo = NULL;

	no_energy = NULL;

	App->tex->UnLoad(bomb);
	App->tex->UnLoad(bzone);
	bomb = NULL;
	bzone = NULL;

	win_background = NULL;
	win_button = NULL;

	loose_background_detected = NULL;
	loose_background_killed = NULL;
	loose_background_hard = NULL;
	loose_button = NULL;

	tutorial_text = NULL;
	tutorial_window = NULL;
	tutorial_button = NULL;
	skip_button = NULL;
	tutorial_image = NULL;
	tutorial_fadeblack = NULL;

	bomb_pos.clear();
	return true;
}

bool GameScene::ExtractionPoint()
{
	list<Unit*>::iterator f_unit = App->entity->friendly_units.begin();
	while (f_unit != App->entity->friendly_units.end())
	{
		iPoint pos = (*f_unit)->GetPosition();

		if (pos.PointInRect(bomb_zone.x, bomb_zone.y, bomb_zone.w, bomb_zone.h) && (*f_unit)->IsVisible())
		{}

		else
			return false;

		++f_unit;
	}

	return true;
}

void GameScene::LoadTutorial()
{
	tutorial_finished = false;

	tutorial_fadeblack = App->ui->CreateImage({ 662, 589, 640, 480 }, 0, 0, true);
	tutorial_window = App->ui->CreateImage({ 1022, 125, 412, 292 }, 102, 85, true);
	tutorial_button = App->ui->CreateButton("", 275, 360, { 847, 137, 53, 23 }, { 847, 163, 53, 23 }, { 847, 111, 53, 23 }, this);
	skip_button = App->ui->CreateButton("", 490, 75, {869, 189, 21, 19}, {891, 189, 21, 19}, { 847, 189, 21, 19 }, this);
	tutorial_text = App->ui->CreateLabel("COLLECT the 3 bombs that the enemies have in their bases.", 140, 300);
	secondary_text = App->ui->CreateLabel(" - Once you got all 3, guide your units to the extraction point", 140, 320);
	tutorial_image = App->ui->CreateImage({ 1300, 475, 69, 63 }, 275, 175, true);

	//Animation
	App->ui->AnimResize(tutorial_window, 0.5f, true, 5);
	App->ui->AnimFade(tutorial_image, 1, true, 6);
	App->ui->AnimFade(tutorial_text, 1, true, 6);
	App->ui->AnimFade(secondary_text, 1, true, 6);
	App->ui->AnimResize(tutorial_button, 0.5f, true, 6.5f);
	App->ui->AnimResize(skip_button, 0.1f, true, 6.5f);

	tutorial_text_queue.push("If firebats NOTICE you, the mission is FAILED.");
	tutorial_text_queue.push("You can eliminate enemy marines. They won't make the call.");
	tutorial_text_queue.push("Target a friendly unit with your medic, it will heal it.");
	tutorial_text_queue.push("Press Q to make the ghost INVISIBLE to all enemies.");
	tutorial_text_queue.push("Press W to activate sniper mode. CLICK RIGHT to shoot.");

	secondary_text_queue.push(" - Firebats have limited range of vision, take advantage of this!");
	secondary_text_queue.push(" - Marines will be patroling the area.");
	secondary_text_queue.push(" - Medics aren't combat units, use them for support purposes!");
	secondary_text_queue.push(" - Using invisibility consumes energy, be careful!");
	secondary_text_queue.push(" - The squad has limited ammo, don't waste it!");

	tutorial_images_queue.push({1435, 500, 300, 281});
	tutorial_images_queue.push({ 1400, 880, 62, 78 });
	tutorial_images_queue.push({1512, 969, 68, 78});
	tutorial_images_queue.push({1403, 968, 96, 79});
	tutorial_images_queue.push({ 1405, 810, 340, 63 });

	LOG("QUEUE SIZE %i", tutorial_images_queue.size());
}


bool GameScene::GamePaused()const
{
	return game_paused;
}

void GameScene::LoadGame(const char* path)
{
	pugi::xml_document	level_file;
	pugi::xml_node		level;

	char* buf;
	int size = App->fs->Load(path, &buf);
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

	if (path == "game_saved.xml")
	{
		App->scene_manager->dificulty = level.child("difficulty").attribute("value").as_bool();
		sniper_ammo = level.child("sniper_ammo").attribute("value").as_int();
	}

	int camera_x = level.child("camera").attribute("x").as_int();
	int camera_y = level.child("camera").attribute("y").as_int();

	App->render->camera = SDL_Rect{ camera_x, camera_y, App->render->camera.w, App->render->camera.h };

	pugi::xml_node bomb_root = level.child("bomb");
	pugi::xml_node bomb_node;
	for (bomb_node = bomb_root.child("position"); bomb_node; bomb_node = bomb_node.next_sibling("position"))
	{
		iPoint bomb_position;
		bomb_position.x = bomb_node.attribute("x").as_int();
		bomb_position.y = bomb_node.attribute("y").as_int();

		bomb_pos.push_back(bomb_position);
	}
	bomb_zone.x = level.child("bomb_zone").attribute("x").as_int();
	bomb_zone.y = level.child("bomb_zone").attribute("y").as_int();
	pugi::xml_node unit_f;
	for (unit_f = level.child("friendly_unit"); unit_f; unit_f = unit_f.next_sibling("friendly_unit"))
	{
		UNIT_TYPE type = App->entity->UnitTypeToEnum(unit_f.child("type").attribute("value").as_string());
		if (App->scene_manager->pro == false || (App->scene_manager->pro == true && type == GHOST))
		{
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

			Unit* u = App->entity->friendly_units.back();

			bool is_selected = unit_f.child("selected").attribute("value").as_bool();
			if (is_selected == true)
			{
				u->Select();
				App->entity->selected_units.push_back(u);
			}

			int life = unit_f.child("life").attribute("value").as_int();
			if (life > 0)
				u->SetLife(life);

			u->direction.x = unit_f.child("direction").attribute("x").as_int();
			u->direction.y = unit_f.child("direction").next_sibling("direction").attribute("y").as_int();
		}
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

	Unit* u = App->entity->enemy_units.back();
	int life = unit_e.child("life").attribute("value").as_int();
	if (life > 0)
		u->SetLife(life);
	u->direction.x = unit_e.child("direction").attribute("x").as_int();
	u->direction.y = unit_e.child("direction").next_sibling("direction").attribute("y").as_int();
	u->original_direction = u->direction;
	}
}

void GameScene::SaveGame(const char* path)
{
	// xml object were we will store all data
	pugi::xml_document data;
	pugi::xml_node root;

	root = data.append_child("level");

	root.append_child("difficulty").append_attribute("value") = App->scene_manager->dificulty;

	root.append_child("sniper_ammo").append_attribute("value") = sniper_ammo;

	root.append_child("camera").append_attribute("x") = App->render->camera.x;
	root.child("camera").append_attribute("y") = App->render->camera.y;

	root.append_child("bomb");
	pugi::xml_node bomb_node = root.child("bomb");

	list<iPoint>::iterator bomb_position = bomb_pos.begin();
	pugi::xml_node bomb_position_node(NULL);

	while (bomb_position != bomb_pos.end())
	{
		bomb_node.append_child("position").append_attribute("x") = (*bomb_position).x;

		if (bomb_position_node == NULL)
		{
			bomb_position_node = bomb_node.child("position");
		}
		else
		{
			bomb_position_node = bomb_position_node.next_sibling("position");
		}

		bomb_position_node.append_attribute("y") = (*bomb_position).y;

		++bomb_position;
	}

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
		int life = (*unit_f)->GetLife();
		friend_unit.append_child("life").append_attribute("value") = life;
		friend_unit.append_child("selected").append_attribute("value") = (*unit_f)->IsSelected();

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
		int life = (*unit_e)->GetLife();
		enemy_unit.append_child("life").append_attribute("value") = life;

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
	App->fs->Save(path, stream.str().c_str(), stream.str().length());
}

void GameScene::OnGUI(UIEntity* gui, GUI_EVENTS event)
{
	if (gui->type == BUTTON)
	{
		if ((UIButton*)gui == quit_button && event == MOUSE_BUTTON_RIGHT_UP)
		{
			App->scene_manager->WantToChangeScene(MENU);
		}

		if ((UIButton*)gui == resume_button && event == MOUSE_BUTTON_RIGHT_UP)
		{
			game_paused = false;

			quit_fadeblack->SetVisible(false);
			//quit_window->SetVisible(false);
			quit_button->SetVisible(false);
			resume_button->SetVisible(false);
		}

		if ((UIButton*)gui == ghost_invisibility_button && event == MOUSE_BUTTON_RIGHT_UP)
		{
			list<Unit*>::iterator it = App->entity->selected_units.begin();
			while (it != App->entity->selected_units.end())
			{
				if ((*it)->GetType() == GHOST)
					(*it)->CastAbility(INVISIBLE);

				++it;
			}
		}

		else if ((UIButton*)gui == ghost_snipermode_button && event == MOUSE_BUTTON_RIGHT_UP)
		{
			list<Unit*>::iterator it = App->entity->selected_units.begin();
			while (it != App->entity->selected_units.end())
			{
				if ((*it)->GetType() == GHOST)
					(*it)->CastAbility(SNIPPER);

				++it;
			}
		}

		else if ((UIButton*)gui == win_button && event == MOUSE_BUTTON_RIGHT_UP)
		{
			App->scene_manager->WantToChangeScene(CREDIT);
		}

		else if ((UIButton*)gui == loose_button && event == MOUSE_BUTTON_RIGHT_UP)
		{
			App->scene_manager->WantToChangeScene(MENU);
		}
		
		else if ((UIButton*)gui == skip_button && event == MOUSE_BUTTON_RIGHT_UP)
		{
			//Animation
			App->ui->AnimResize(tutorial_window, 0.05f, false, 1.0f);
			App->ui->AnimFade(tutorial_image, 0.5f, false, 0.5f);
			App->ui->AnimFade(tutorial_text, 0.5f, false, 0.5f);
			App->ui->AnimFade(secondary_text, 0.5f, false, 0.5f);
			App->ui->AnimResize(tutorial_button, 0.05f, false);
			App->ui->AnimResize(skip_button, 0.05f, false);
			App->ui->AnimFade(tutorial_fadeblack, 1.5f, false, 2);

			queue<SDL_Rect> empty_sdl;
			queue<char*> empty_char;
			tutorial_text_queue = empty_char;
			tutorial_images_queue = empty_sdl;
			secondary_text_queue = empty_char;

			tutorial_finished = true;
			game_paused = false;
		}

		else if ((UIButton*)gui == tutorial_button && event == MOUSE_BUTTON_RIGHT_UP)
		{
			if (tutorial_text_queue.size() > 0)
			{
				//Animation
				App->ui->AnimFade(secondary_text, 1, true);
				App->ui->AnimFade(tutorial_text, 1, true);
				App->ui->AnimFade(tutorial_image, 1, true);

				secondary_text->Print(secondary_text_queue.front());
				secondary_text_queue.pop();
				tutorial_text->Print(tutorial_text_queue.front());
				tutorial_text_queue.pop();
				
				switch (tutorial_images_queue.size())
				{
				case 5:
					tutorial_image->SetLocalPos(170, 45);
					break;
				case 4:
					tutorial_image->SetLocalPos(270, 175);
					break;
				case 3:
					tutorial_image->SetLocalPos(270, 175);
					break;
				case 2:
					tutorial_image->SetLocalPos(250, 175);
					break;
				case 1:
					tutorial_image->SetLocalPos(145, 175);
					break;
				}
				tutorial_image->SetImageRect(tutorial_images_queue.front());
				tutorial_images_queue.pop();
			}
			else
			{

				//Animation
				App->ui->AnimResize(tutorial_window, 0.05f, false, 1.0f);
				App->ui->AnimFade(tutorial_image, 0.5f, false, 0.5f);
				App->ui->AnimFade(tutorial_text, 0.5f, false, 0.5f);
				App->ui->AnimFade(secondary_text, 0.5f, false, 0.5f);
				App->ui->AnimResize(tutorial_button, 0.05f, false);
				App->ui->AnimResize(skip_button, 0.05f, false);
				App->ui->AnimFade(tutorial_fadeblack, 1.5f, false, 2);
				
				tutorial_finished = true;
				game_paused = false;
			}

		}
	}
}

void GameScene::SelectFX(UNIT_TYPE type)
{
	switch (type)
	{
	case MARINE:
		App->audio->PlayFx(marine_select);
		break;
	case GHOST:
		App->audio->PlayFx(ghost_select);
		break;
	case FIREBAT:
		App->audio->PlayFx(firebat_select);
		break;
	case MEDIC:
		App->audio->PlayFx(medic_select);
		break;
	case OBSERVER:
		App->audio->PlayFx(observer_select);
		break;
	}
}

void GameScene::MoveFX(UNIT_TYPE type)
{
	switch (type)
	{
	case MARINE:
		App->audio->PlayFx(marine_order);
		break;
	case GHOST:
		App->audio->PlayFx(ghost_order);
		break;
	case FIREBAT:
		App->audio->PlayFx(firebat_order);
		break;
	case MEDIC:
		App->audio->PlayFx(medic_order);
		break;
	case OBSERVER:
		App->audio->PlayFx(observer_order);
		break;
	}
}

void GameScene::AttackFX(UNIT_TYPE type)
{
	switch (type)
	{
	case MARINE:
		App->audio->PlayFx(marine_attack_order);
		break;
	case GHOST:
		App->audio->PlayFx(ghost_attack_order);
		break;
	case FIREBAT:
		App->audio->PlayFx(firebat_attack_order);
		break;
	case MEDIC:
		App->audio->PlayFx(medic_attack_order);
		break;
	}
}

void GameScene::LoadAudio()
{
	//Load all fx uint sounds
	//Select
	marine_select = App->audio->LoadFx("FX/Terran/Marine/PieceOfMe.wav");
	ghost_select = App->audio->LoadFx("FX/Terran/Ghost/ImHere.wav");
	firebat_select = App->audio->LoadFx("FX/Terran/Firebat/GoodSmoke.wav");
	medic_select = App->audio->LoadFx("FX/Terran/Medic/MedicalAttention.wav");
	observer_select = App->audio->LoadFx("FX/protoss/probe/pprerr00.wav");


	//order
	marine_order = App->audio->LoadFx("FX/Terran/Marine/RockndRoll.wav");
	ghost_order = App->audio->LoadFx("FX/Terran/Ghost/Gone.wav");
	firebat_order = App->audio->LoadFx("FX/Terran/Firebat/GotIt.wav");
	medic_order = App->audio->LoadFx("FX/Terran/Medic/OnTheJob.wav");
	observer_order = App->audio->LoadFx("FX/protoss/probe/ppryes02.wav");

	//attack order
	marine_attack_order = App->audio->LoadFx("FX/Terran/Marine/GoGoGo.wav");
	ghost_attack_order = App->audio->LoadFx("FX/Terran/Ghost/CallShot.wav");
	firebat_attack_order = App->audio->LoadFx("FX/Terran/Firebat/LetsBurn.wav");
	medic_attack_order = App->audio->LoadFx("FX/Terran/Medic/SpongeBath.wav");
	//observer_order_attack_order;

	//death
	marine_death = App->audio->LoadFx("FX/Terran/Marine/tmadth00.wav");
	ghost_death = App->audio->LoadFx("FX/Terran/Ghost/tghdth01.wav");
	firebat_death = App->audio->LoadFx("FX/Terran/Firebat/tfbdth02.wav");
	medic_death = App->audio->LoadFx("FX/Terran/Medic/tmddth00.wav");
	observer_death = App->audio->LoadFx("FX/protoss/probe/pprdth00.wav");

	//Setting fx volume
	App->audio->SetFxVolume(15);
	App->audio->SetFxVolume(6, "sounds/marine_shot.ogg");
	App->audio->SetFxVolume(6, "sounds/ghost_shot.ogg");
	App->audio->SetFxVolume(6, "sounds/firebat_shot.ogg");

}

void GameScene::LoadHUD()
{
	//Create HUD
	App->ui->CreateImage({ 0, 397, 25, 187 }, 0, 294, true);
	App->ui->CreateImage({ 25, 418, 143, 166 }, 25, 315, true);
	App->ui->CreateImage({ 166, 471, 234, 112 }, 166, 368, true);
	App->ui->CreateImage({ 400, 477, 46, 107 }, 400, 374, true);
	App->ui->CreateImage({ 446, 457, 22, 127 }, 446, 354, true);
	App->ui->CreateImage({ 466, 436, 21, 148 }, 466, 333, true);
	App->ui->CreateImage({ 487, 429, 157, 155 }, 487, 326, true);

	//Creating Mini Map
	minimap = App->ui->CreateMiniMap({ 5, 345, 130, 130 }, { 867, 442, 130, 130 }, { 4096, 4096 });

	
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

	//Create GHOST Buttons
	ghost_invisibility_button = App->ui->CreateButton("", 508, 358, { 947, 189, 33, 34 }, { 947, 259, 33, 34 }, { 947, 224, 33, 34 }, this);
	ghost_invisibility_button->SetVisible(false);
	ghost_snipermode_button = App->ui->CreateButton("", 553, 358, { 913, 189, 33, 34 }, { 913, 259, 33, 34 }, { 913, 224, 33, 34 }, this);
	ghost_snipermode_button->SetVisible(false);

	//CREATE EVENT
	objectives_box = App->ui->CreateImage(SDL_Rect{ 0, 90, 169, 71 }, 470, -5, true);
	//objective_info_1 = App->ui->CreateLabel("You must retrieve the bomb ", 477, 5);
	//objective_info_2 = App->ui->CreateLabel("from the southern enemy", 477, 15);
	//objective_info_3 = App->ui->CreateLabel("base", 477, 25);
	pause_mark = App->ui->CreateImage(SDL_Rect{ 66, 162, 56, 38 }, 470 - 56, 0, false);
	run_mark = App->ui->CreateImage(SDL_Rect{ 0, 162, 56, 38 }, 470 - 56, 0, true);
	sniper_ammo_label = App->ui->CreateLabel("Cal. 50 bullets: 3", 485, 7);
	if (App->scene_manager->level_saved == true)
	{
		char ui_sniper_ammo[20];
		sprintf_s(ui_sniper_ammo, sizeof(ui_sniper_ammo), "Cal. 50 bullets: %d", sniper_ammo);
		sniper_ammo_label->Print(ui_sniper_ammo, false);
	}
	grenade_ammo_label = App->ui->CreateLabel("Bombs left: 3", 485, 22);

	//Pathfinding Label
	pathfinding_label = App->ui->CreateLabel("I can't go there Sir!", 100, 50, true);
	pathfinding_label->is_visible = false;

	game_saved = App->ui->CreateLabel("Game Saved", 300, 200, true);
	game_saved->is_visible = false;

	App->events->game_event = INITIAL_STATE;

	//Life in HUD
	//life_HUD = App->ui->CreateLabel("", 177, 458);

	//Sniper
	snipper_ui = App->ui->CreateImage({ 0, 651, 640, 480 }, 0, 0, false);

	//Ammo
	no_ammo = App->ui->CreateLabel("Out of ammo!", 280, 300, true);
	no_ammo->is_visible = false;

	//Energy
	no_energy = App->ui->CreateLabel("Not enough energy!", 280, 350, true);
	no_energy->is_visible = false;

	//Win image & button
	win_background = App->ui->CreateImage({ 847, 1, 221, 108 }, 220, 150, false, true);
	win_button = App->ui->CreateButton("", 304, 246, { 847, 137, 53, 23 }, { 847, 163, 53, 23 }, { 847, 111, 53, 23 }, this);
	//win_button->SetParent(win_background);
	win_button->SetVisible(false);

	//Loose image & button
	loose_background_detected = App->ui->CreateImage({ 1073, 1, 221, 108 }, 220, 150, false, true);
	loose_background_killed = App->ui->CreateImage({ 1835, 966, 221, 108 }, 220, 150, false, true);
	loose_background_hard = App->ui->CreateImage({ 2081, 963, 221, 108 }, 220, 150, false, true);
	loose_button = App->ui->CreateButton("", 304, 246, { 902, 137, 53, 23 }, { 902, 163, 53, 23 }, { 902, 111, 53, 23 }, this);
	
	//loose_button->SetParent(loose_background_detected);
	loose_button->SetVisible(false);
}

void GameScene::LoseGameDetected()
{
	if (game_finished == false)
	{
		loose_background_detected->is_visible = true;
		loose_button->is_visible = true;
		game_paused = true;

		game_finished = true;

		App->ui->AnimResize(loose_background_detected, 0.5f, true);
		App->ui->AnimResize(loose_button, 0.25f, true, 0.3f);
	}
}
void GameScene::LoseGameKilled()
{

	if (game_finished == false)
	{
		if (App->scene_manager->dificulty == false)
		{
			loose_background_killed->is_visible = true;
			loose_button->is_visible = true;
			game_paused = true;

			game_finished = true;

			App->ui->AnimResize(loose_background_killed, 0.5f, true);
			App->ui->AnimResize(loose_button, 0.25f, true, 0.3f);
		}

		else
		{
			loose_background_hard->is_visible = true;
			loose_button->is_visible = true;
			game_paused = true;

			game_finished = true;

			App->ui->AnimResize(loose_background_hard, 0.5f, true);
			App->ui->AnimResize(loose_button, 0.25f, true, 0.3f);
		}
		
	}
}

bool GameScene::GetFinishedGame()
{
	return game_finished;
}

bool GameScene::GetTutorialState()
{
	return tutorial_finished;
}

bool GameScene::IsGhostSelected()
{
	bool ret = false;

	list<Unit*>::iterator it = App->entity->selected_units.begin();
	while (it != App->entity->selected_units.end())
	{
		if ((*it)->GetType() == GHOST)
			ret = true;

		++it;
	}

	return ret;
}

void GameScene::LoadQuitUI()
{
	quit_fadeblack = App->ui->CreateImage({ 662, 589, 640, 480 }, 0, 0, false, true);
	//quit_window = App->ui->CreateImage({ 1022, 125, 412, 292 }, 102, 85, false, true);
	quit_button = App->ui->CreateButton("QUIT", 250, 250, { 348, 109, 125, 26 }, { 348, 161, 125, 26 }, { 348, 135, 125, 26 }, this);
	quit_button->SetVisible(false);
	resume_button = App->ui->CreateButton("RESUME", 250, 180, { 348, 109, 125, 26 }, { 348, 161, 125, 26 }, { 348, 135, 125, 26 }, this);
	resume_button->SetVisible(false);
}