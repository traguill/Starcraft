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
#include "DevScene.h"
#include "CreditScene.h"
#include "InputManager.h"

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
	
	//Disable credit scene
	App->credit_scene->DisableModule();

	//Disable dev_scene
	App->dev_scene->DisableModule();

	return ret;
}

// Called before the first frame
bool SceneManager::Start()
{
	actual_scene = MENU;

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
		DisableScene(actual_scene);
		EnableScene(new_scene);

		actual_scene = new_scene;
		changing_scene = false;
	}

	return ret;
}

// Called before quitting
bool SceneManager::CleanUp()
{
	LOG("Freeing Scene Manager");



	return true;
}

void SceneManager::WantToChangeScene(SCENES scene)
{
	changing_scene = true;

	new_scene = scene;
}

void SceneManager::DisableScene(SCENES scene)
{
	switch (scene)
	{
	case MENU:
		DisableMenu();
		break;
	case GAME:
		DisableGame();
		break;
	case DEV:
		DisableDev();
		break;
	case CREDIT:
		DisableCredit();
		break;
	}
}

void SceneManager::EnableScene(SCENES scene)
{
	switch (scene)
	{
	case MENU:
		EnableMenu();
		break;
	case GAME:
		EnableGame();
		break;
	case DEV:
		EnableDev();
		break;
	case CREDIT:
		EnableCredit();
		break;
	}
}

void SceneManager::EnableMenu()
{
	App->menu->EnableModule();
	App->menu->Start();
}

void SceneManager::DisableMenu()
{
	App->menu->DisableModule();

	App->ui->CleanUpList();
	App->menu->CleanUp();
}

void SceneManager::EnableGame()
{
	App->game_scene->EnableModule();
	App->entity->EnableModule();
	App->tactical_ai->EnableModule();
	App->events->EnableModule();

	App->ui->StartGameUI();
	App->input_manager->Start();

	App->entity->Start();
	App->game_scene->Start();
	App->tactical_ai->Start();
	App->events->Start();

	in_game = true;
}

void SceneManager::DisableGame()
{
	App->tactical_ai->DisableModule();
	App->entity->DisableModule();
	App->game_scene->DisableModule();
	App->events->DisableModule();

	App->ui->CleanUpList();
	App->ui->CleanUpGameUI();

	App->events->CleanUp();
	App->tactical_ai->CleanUp();
	App->entity->CleanUp();
	App->game_scene->CleanUp();
	
	in_game = false;
}

void SceneManager::EnableDev()
{
	App->dev_scene->EnableModule();
	App->entity->EnableModule();


	App->entity->Start();
	App->dev_scene->Start();

}

void SceneManager::DisableDev()
{
	App->dev_scene->DisableModule();
	App->entity->DisableModule();

	App->entity->CleanUp();
	App->dev_scene->CleanUp();


}

void SceneManager::EnableCredit()
{
	App->credit_scene->EnableModule();
	App->credit_scene->Start();
}

void SceneManager::DisableCredit()
{
	App->credit_scene->DisableModule();

	App->ui->CleanUpList();
	App->credit_scene->CleanUp();
}