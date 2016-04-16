#include "EventsManager.h"
#include "j1App.h"
#include "j1Module.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "GameScene.h"
#include "j1Input.h"


EventsManager::EventsManager() : j1Module() { }

// Destructor
EventsManager::~EventsManager(){ }

// Called when before render is available
bool EventsManager::Awake(pugi::xml_node&){ return true; }

// Call before first frame
bool EventsManager::Start()
{
	return true;
}

// Called before all Updates
bool EventsManager::PreUpdate(){ return true; }

bool EventsManager::Update(float dt)
{
	switch (game_event)
	{
	case BOMB_RETRIVED:
		BombRetrieved();
		break;
	}


	

	if (App->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN)
		game_event = BOMB_RETRIVED;

	return true;
}

// Called after all Updates
bool EventsManager::PostUpdate(){ return true; }

// Called before quitting
bool EventsManager::CleanUp(){ return true; }

void EventsManager::BombRetrieved()
{
	App->game_scene->objective_info_1->Print("Plant the bomb at their");
	App->game_scene->objective_info_2->Print("main base. It is to");
	App->game_scene->objective_info_3->Print("the south");
}