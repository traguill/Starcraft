#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "SceneManager.h"
#include "GameScene.h"
#include "EntityManager.h"
#include "MenuScene.h"
#include "j1Input.h"

SceneManager::SceneManager() : j1Module()
{
	name.append("scene_manager");
}

// Destructor
SceneManager::~SceneManager()
{}

// Called before render is available
bool SceneManager::Awake(pugi::xml_node& conf)
{
	LOG("Loading SceneManager");
	bool ret = true;

	//Disable in-game scenes
	App->game_scene->DisableModule();
	App->entity->DisableModule();
	App->tactical_ai->DisableModule();

	return ret;
}

// Called before the first frame
bool SceneManager::Start()
{


	return true;
}

// Called each loop iteration
bool SceneManager::PreUpdate()
{

	return true;
}

// Called each loop iteration
bool SceneManager::Update(float dt)
{

	if (App->input->GetKey(SDL_SCANCODE_KP_1) == KEY_UP)
	{
		StartGame();
	}

	if (App->input->GetKey(SDL_SCANCODE_KP_2) == KEY_UP)
	{
		StartMenu();
	}

	return true;
}

// Called each loop iteration
bool SceneManager::PostUpdate()
{
	bool ret = true;

	if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

// Called before quitting
bool SceneManager::CleanUp()
{
	LOG("Freeing Scene Manager");



	return true;
}


void SceneManager::StartGame()
{
	if (in_game == false)
	{
		App->menu->DisableModule();

		App->game_scene->EnableModule();
		App->entity->EnableModule();
		App->tactical_ai->EnableModule();

		App->game_scene->Start();
		App->entity->Start();
		App->tactical_ai->Start();

		in_game = true;
	}

}

void SceneManager::StartMenu()
{
	if (in_game == true)
	{
		App->tactical_ai->DisableModule();
		App->entity->DisableModule();
		App->game_scene->DisableModule();

		App->menu->EnableModule();
		App->menu->Start();

		in_game = false;
	}

}