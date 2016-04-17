#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "SceneManager.h"
#include "GameScene.h"
#include "EntityManager.h"
#include "MenuScene.h"
#include "j1Input.h"
#include "j1UIManager.h"
#include "EventsManager.h"

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


	return true;
}

// Called each loop iteration
bool SceneManager::PostUpdate()
{
	bool ret = true;

	if (changing_scene == true)
	{
		if (in_game == false)
			StartGame();

		else
			StartMenu();
	}

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

void SceneManager::WantToChangeScene()
{
	changing_scene = true;
}

void SceneManager::StartGame()
{
	App->menu->DisableModule();

	App->game_scene->EnableModule();
	App->entity->EnableModule();
	App->tactical_ai->EnableModule();
	App->events->EnableModule();

	App->ui->CleanUpList();

	App->entity->Start();
	App->game_scene->Start();
	App->tactical_ai->Start();

	in_game = true;
	changing_scene = false;
}

void SceneManager::StartMenu()
{
	App->tactical_ai->DisableModule();
	App->entity->DisableModule();
	App->game_scene->DisableModule();
	App->events->DisableModule();

	App->ui->CleanUpList();
	App->entity->CleanUpList();
	//App->ui->CleanUpBars();

	App->menu->EnableModule();
	App->menu->Start();

	in_game = false;
	changing_scene = false;
}