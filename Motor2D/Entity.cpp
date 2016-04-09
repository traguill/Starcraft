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
void Entity::Update(float dt)
{
}

void Entity::Draw()
{
	
}

bool Entity::CleanUp()
{
	return true;
}

void Entity::SetPosition(int x, int y)
{
	logic_pos.x = x;
	logic_pos.y = y;
}

iPoint Entity::GetPosition()const
{
	return logic_pos;
}

SDL_Rect Entity::GetCollider()
{
	collider.x = logic_pos.x - (collider.w * 0.5f);
	collider.y = logic_pos.y - (collider.h * 0.5f);

	return collider;
}

iPoint Entity::GetDrawPosition()
{
	sprite.position.x = logic_pos.x - (collider.w / 2);
	sprite.position.y = logic_pos.y - (height - (collider.w / 2));

	return sprite.position;
}


