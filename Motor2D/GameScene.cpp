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

GameScene::GameScene() : j1Module()
{
	name.append("scene");
}

// Destructor
GameScene::~GameScene()
{}

// Called before render is available
bool GameScene::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

	return ret;
}

// Called before the first frame
bool GameScene::Start()
{
	//Load Map
	App->map->Load("game_map.tmx", map_id);

	//Load collision map
	if (App->map->Load("collision.tmx", collider_id) == true)
	{
		int width, height;
		uchar* buffer = NULL;
		App->map->CreateWalkabilityMap(width, height, &buffer, collider_id);

		App->pathfinding->SetMap(width, height, buffer);
		RELEASE_ARRAY(buffer);
	}


	debug = false;
	game_paused = false;

	//UI TESTS
	App->ui->CreateImage({ 1, 194, 599, 155 }, 21, 325);
	//
	//Not able to use labels, missing font;
	//----------App->ui->CreateLabel("hola", 100, 100);
	//----------App->ui->CreateButton("hola", 100, 100, { 0,0,50,50 }, { 0, 0, 0, 500 }, { 0, 0, 0, 0 });
	vector<SDL_Rect> sections;
	sections.push_back({ 1, 62, 20, 21 });
	sections.push_back({ 22, 62, 20, 21 });
	sections.push_back({ 43, 62, 20, 21 });
	sections.push_back({ 64, 62, 20, 21 });
	sections.push_back({ 85, 62, 20, 21 });
	main_cur = App->ui->CreateCursor(sections, 0.08);
	


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

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;
	if (debug)
		App->map->Draw(collider_id);

	if (App->input->GetKey(SDL_SCANCODE_P) == KEY_DOWN)
		game_paused = !game_paused;


	return true;
}

// Called each loop iteration
bool GameScene::PostUpdate()
{
	bool ret = true;

	if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

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