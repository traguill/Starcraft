#include "EventsManager.h"
#include "j1App.h"
#include "j1Module.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Input.h"


EventsManager::EventsManager() : j1Module() { }

// Destructor
EventsManager::~EventsManager(){ }

// Called when before render is available
bool EventsManager::Awake(pugi::xml_node&){ return true; }

// Call before first frame
bool EventsManager::Start()
{
	objectives_box = App->ui->CreateImage(SDL_Rect{ 0, 90, 169, 66 }, 470, -5);
	objective_info_1 = App->ui->CreateLabel("You must retrieve the bomb ", 477, 5);
	objective_info_2 = App->ui->CreateLabel("from the western enemy", 477, 15);
	objective_info_3 = App->ui->CreateLabel("base", 477, 25);
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

	if (App->input->GetKey(SDL_SCANCODE_P) == KEY_DOWN)
		game_event = BOMB_RETRIVED;

	return true;
}

// Called after all Updates
bool EventsManager::PostUpdate(){ return true; }

// Called before quitting
bool EventsManager::CleanUp(){ return true; }

void EventsManager::BombRetrieved()
{
	objective_info_1->Print("Plant the bomb at their");
	objective_info_2->Print("main base. It is to");
	objective_info_3->Print("the south");
}