#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "p2Point.h"
#include "SDL/include/SDL.h"
#include <list>

using namespace std;

class j1Module;

class Entity
{
	friend class j1EntityManager;	//Provisional

public:

	Entity();

	// Entity
	virtual ~Entity();

	virtual void Update(float dt);
	// Called before quitting

	virtual void Draw();

	virtual bool CleanUp();

protected:

	SDL_Texture* texture;
	iPoint pos;
	SDL_Rect* collider;
	int life;

};

#endif