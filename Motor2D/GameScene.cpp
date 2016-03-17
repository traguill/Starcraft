#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1Map.h"
//#include "j1PathFinding.h"
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
	App->map->Load("game_map.tmx");

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
	App->map->Draw();
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
