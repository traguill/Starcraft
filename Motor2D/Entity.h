#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "p2Point.h"
#include "j1Textures.h"
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

	virtual void Draw();

	virtual bool CleanUp();

	//Logic position
	void SetPosition(int x, int y); 
	iPoint GetPosition()const;  

	SDL_Rect GetCollider();
	iPoint GetDrawPosition();


protected:

	Sprite sprite;
	int life;
	int friendly_life;
	int max_life;
	int friendly_max_life;

	SDL_Rect collider;
	iPoint logic_pos;

public:
	int width;
	int height;

};

#endif