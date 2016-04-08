#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "p2Point.h"
#include "j1Textures.h"
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

	virtual void Draw();

	virtual bool CleanUp();

	//Logic position
	void SetPosition(int x, int y); 
	iPoint GetPosition()const;  

	SDL_Rect GetCollider();
	iPoint GetDrawPosition();


protected:

	Sprite sprite;
	SDL_Texture* auxiliar_texture;
	int life;

	int width;
	int height;


	SDL_Rect collider;
	iPoint logic_pos;

};

#endif