#include "Projectile.h"

Projectile::Projectile(Projectile* p)
{
	sprite.texture = p->sprite.texture;

	sprite.rect.w = p->sprite.rect.w;
	sprite.rect.h = p->sprite.rect.h;

	up = p->up;
	up.speed = p->anim_speed;
	down = p->down;
	down.speed = p->anim_speed;
	right = p->right;
	right.speed = p->anim_speed;
	left = p->left;
	left.speed = p->anim_speed;
	up_right = p->up_right;
	up_right.speed = p->anim_speed;
	down_right = p->down_right;
	down_right.speed = p->anim_speed;
	up_left = p->up_left;
	up_left.speed = p->anim_speed;
	down_left = p->down_left;
	down_left.speed = p->anim_speed;

	pos_up = p->pos_up;
	pos_down = p->pos_down;
	pos_right = p->pos_right;
	pos_left = p->pos_left;
	pos_up_right = p->pos_up_right;
	pos_down_right = p->pos_down_right;
	pos_up_left = p->pos_up_left;
	pos_down_left = p->pos_down_left;


	current_pos = { 0, 0 };
	current_animation = NULL;
}