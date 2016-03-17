#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Fonts.h"
#include "j1Input.h"
#include "EntityManager.h"
#include "UILabel.h"
#include "UIImage.h"
#include "UIButton.h"
#include "j1Window.h"
#include "UIInputBox.h"


j1EntityManager::j1EntityManager() : j1Module()
{
	
}

// Destructor
j1EntityManager::~j1EntityManager()
{}

// Called before render is available
bool j1EntityManager::Awake(pugi::xml_node& conf)
{
	bool ret = true;


	return ret;
}

// Called before the first frame
bool j1EntityManager::Start()
{
	bool ret = true;

	return ret;
}

// Update all UIManagers
bool j1EntityManager::PreUpdate()
{
	return true;
}

bool j1EntityManager::Update(float dt)
{
	map<string, Entity*>::iterator it = friendly_units.begin();
	while (it != friendly_units.end())
	{
		(*it).second->Update(dt);
	}

	list<Entity*>::iterator i = hostile_enities.begin();
	while (i != hostile_enities.end())
	{
		(*i)->Update(dt);
	}

	return true;
}

// Called after all Updates
bool j1EntityManager::PostUpdate()
{
	map<string, Entity*>::iterator it = friendly_units.begin();
	while (it != friendly_units.end())
	{
		(*it).second->Draw();
	}

	list<Entity*>::iterator i = hostile_enities.begin();
	while (i != hostile_enities.end())
	{
		(*i)->Draw();
	}

	return true;
}

bool j1EntityManager::CleanUp()
{
	return true;
}