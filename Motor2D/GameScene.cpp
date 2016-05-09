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

	//Ammunition set
	sniper_ammo = 3;
	intel_left = 3;

	LoadHUD();

	//Bomb
	bomb = App->tex->Load("sprites/Bomb.png");
	bomb_rect = { 30, 12, 32, 32 };
	bomb_zone = { 815, 1780, 50, 50 };
	bomb_pos.clear();

	debug = false;
	game_paused = false;

	//UI TESTS
	//
	//Not able to use labels, missing font;
	//----------App->ui->CreateLabel("hola", 100, 100);
	//----------App->ui->CreateButton("hola", 100, 100, { 0,0,50,50 }, { 0, 0, 0, 500 }, { 0, 0, 0, 0 });
	
	//music = App->audio->PlayMusic("StarcraftTerrantheme1.wav");

	LoadLevel("my_level.xml");
	LoadAudio();

	App->render->camera = SDL_Rect{ -700, -150, App->render->camera.w, App->render->camera.h };


	game_finished = false;

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

	//Debug (go to menu)
	if (App->input->GetKey(SDL_SCANCODE_KP_1) == KEY_UP)
		App->scene_manager->WantToChangeScene(MENU);

	App->map->Draw(map_id);


	//Updating timer
	if (parthfinding_label_timer.ReadSec() >= 3)
	{
		pathfinding_label->is_visible = false;
	}

	if (sniper_ui_timer.ReadSec() >= 3)
	{
		no_energy->is_visible = false;
		no_ammo->is_visible = false;
	}


	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;
	if (debug)
		App->map->Draw(collider_id);

	if (App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN)
		game_paused = !game_paused;


	//Save level designed
	/*if (App->input->GetKey(SDL_SCANCODE_KP_8) == KEY_UP)
	{
		SaveLevelDesign();
	}*/





	//Check win or lose
	if (App->entity->friendly_units.size() == 0)
	{
		LoseGame();
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
				if (pos.x > (*bomb_position).x && pos.x < (*bomb_position).x + bomb_rect.w && pos.y > (*bomb_position).y && pos.y < (*bomb_position).y + bomb_rect.w && (*f_unit)->IsVisible())
				{
					bomb_position = bomb_pos.erase(bomb_position);
				}
				++bomb_position;
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

	char ui_sniper_ammo[20];
	sprintf_s(ui_sniper_ammo, sizeof(ui_sniper_ammo), "Cal. 50 bullets: %d", sniper_ammo);
	sniper_ammo_label->Print(ui_sniper_ammo, false);

	char ui_intel_left[20];
	sprintf_s(ui_intel_left, sizeof(ui_intel_left), "Intel left: %d", intel_left);
	grenade_ammo_label->Print(ui_intel_left, false);

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
		App->render->DrawQuad(bomb_zone, 255, 255, 0, 125, true, true);

	if (GamePaused())
	{
		run_mark->is_visible = false;
		pause_mark->is_visible = true;

		if (active_timer == false)
		{
			pause_timer.Start();
			active_timer = true;
		}

		if (pause_timer.ReadSec() <= 0.8)
		{
			App->render->DrawQuad(SDL_Rect{ -App->render->camera.x + 1, -App->render->camera.y + 1, App->render->camera.w - 2, App->render->camera.h - 2 }, 255, 0, 0, 255, false, true);
			App->render->DrawQuad(SDL_Rect{ -App->render->camera.x + 2, -App->render->camera.y + 2, App->render->camera.w - 4, App->render->camera.h - 4 }, 255, 0, 0, 255, false, true);
			App->render->DrawQuad(SDL_Rect{ -App->render->camera.x + 3, -App->render->camera.y + 3, App->render->camera.w - 6, App->render->camera.h - 6 }, 255, 0, 0, 255, false, true);
			App->render->DrawQuad(SDL_Rect{ -App->render->camera.x + 4, -App->render->camera.y + 4, App->render->camera.w - 8, App->render->camera.h - 8 }, 255, 0, 0, 255, false, true);
		}

		if (pause_timer.ReadSec() > 1.6)
			active_timer = false;
	}

	else
	{
		run_mark->is_visible = true;
		pause_mark->is_visible = false;
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

	life_HUD = NULL;
	
	marine_weapon_icon = NULL;
	marine_armour_icon = NULL;
	marine_wireframe = NULL;

	ghost_weapon_icon = NULL;
	ghost_wireframe = NULL;;

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
	bomb = NULL;

	win_background = NULL;
	win_button = NULL;

	loose_background = NULL;
	loose_button = NULL;

	bomb_pos.clear();
	return true;
}


bool GameScene::GamePaused()const
{
	return game_paused;
}

void GameScene::LoadLevel(const char* path)
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

void GameScene::SaveLevelDesign(const char* path)
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
	App->fs->Save(path, stream.str().c_str(), stream.str().length());
	
}

void GameScene::OnGUI(UIEntity* gui, GUI_EVENTS event)
{
	if (gui->type == BUTTON)
	{
		if ((UIButton*)gui == win_button && event == MOUSE_BUTTON_RIGHT_UP)
		{
			App->scene_manager->WantToChangeScene(MENU);
		}

		else if ((UIButton*)gui == loose_button && event == MOUSE_BUTTON_RIGHT_UP)
		{
			App->scene_manager->WantToChangeScene(MENU);
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
	//Setting fx volume
	App->audio->SetFxVolume(10);
	App->audio->SetFxVolume(5, "sounds/marine_shot.ogg");
	App->audio->SetFxVolume(5, "sounds/ghost_shot.ogg");
	App->audio->SetFxVolume(5, "sounds/firebat_shot.ogg");

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
	App->ui->CreateMiniMap({ 5, 345, 130, 130 }, { 867, 442, 130, 130 }, { 4096, 4096 });


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
	pause_mark = App->ui->CreateImage(SDL_Rect{ 66, 162, 56, 38 }, 470 - 56, 0, false);
	run_mark = App->ui->CreateImage(SDL_Rect{ 0, 162, 56, 38 }, 470 - 56, 0, true);
	sniper_ammo_label = App->ui->CreateLabel("cal. 50 bullets: 3", 477, 50);
	grenade_ammo_label = App->ui->CreateLabel("hand grenades: 3", 477, 62);

	//Pathfinding Label
	pathfinding_label = App->ui->CreateLabel("I can't go there Sir!", 100, 50, true);
	pathfinding_label->is_visible = false;

	App->events->game_event = INITIAL_STATE;

	//Life in HUD
	life_HUD = App->ui->CreateLabel("", 177, 458);

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
	loose_background = App->ui->CreateImage({ 1073, 1, 221, 108 }, 220, 150, false, true);
	loose_button = App->ui->CreateButton("", 304, 246, { 902, 137, 53, 23 }, { 902, 163, 53, 23 }, { 902, 111, 53, 23 }, this);
	
	//loose_button->SetParent(loose_background);
	loose_button->SetVisible(false);
}

void GameScene::LoseGame()
{
	if (game_finished == false)
	{
		loose_background->is_visible = true;
		loose_button->is_visible = true;
		game_paused = true;

		game_finished = true;
	}
}