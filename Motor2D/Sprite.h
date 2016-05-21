#ifndef __SPRITE_H__
#define __SPRITE_H__

#include "p2Point.h"
#include "SDL\include\SDL.h"

using namespace std;

class Sprite
{
public:

	Sprite() : texture(NULL), alpha(255)
	{}
	~Sprite()
	{
		texture = NULL;
	}

public:
	iPoint		 position;
	SDL_Texture* texture;
	SDL_Rect	 rect;
	int			 alpha = 255;

};

#endif