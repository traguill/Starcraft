#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "p2Point.h"
#include "SDL/include/SDL.h"
#include <list>

using namespace std;

class j1Module;

enum UNIT_TYPE
{
};

class Entity
{
public:

	Entity();

	// Entity
	virtual ~Entity();

	virtual bool Update(float dt);
	// Called before quitting

	virtual void Draw();

	virtual bool CleanUp();

private:

	SDL_Texture* texture;
	p2Point<uint> pos;
	SDL_Rect* collider;
	bool selected;
	int life;

};

#endif