#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "Animation.h"
#include "p2Point.h"
#include "j1Textures.h"

struct Projectile
{
	Sprite sprite;

	float anim_speed;

	iPoint pos_up;
	iPoint pos_down;
	iPoint pos_right;
	iPoint pos_left;
	iPoint pos_up_right;
	iPoint pos_down_right;
	iPoint pos_up_left;
	iPoint pos_down_left;

	Animation up;
	Animation down;
	Animation right;
	Animation left;
	Animation up_right;
	Animation down_right;
	Animation up_left;
	Animation down_left;

	iPoint current_pos;
	Animation* current_animation;

	Projectile(){}

	Projectile(Projectile* p);

	~Projectile()
	{
		sprite.texture = NULL;
		current_animation = NULL;
	}
};
#endif