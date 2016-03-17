#include "Entity.h"
#include "j1App.h"
#include "j1Input.h"
#include "p2Log.h"
#include "j1Module.h"
#include "j1Render.h"

Entity::Entity()
{}

// Destructor
Entity::~Entity()
{}

// Called before render is available
bool Entity::Update(float dt)
{
	return true;
}

void Entity::Draw()
{
	
}

bool Entity::CleanUp()
{
	return true;
}