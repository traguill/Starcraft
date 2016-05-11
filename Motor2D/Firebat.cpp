#include "Unit.h"
#include "Firebat.h"
#include "j1Render.h"
#include "j1App.h"
#include "EntityManager.h"

Firebat::Firebat() : Unit()
{

}

Firebat::Firebat(Unit* unit, Projectile* projectile) : Unit(unit, false)
{
	p = projectile;
	p->current_animation = &p->up;

	auxiliar_texture = unit->GetAuxiliarTexture();
	walk_anim_speed = unit->walk_anim_speed;
	idle_anim_speed = unit->idle_anim_speed;
	attack_anim_speed = unit->attack_anim_speed;
	death_anim_speed = unit->death_anim_speed;
}

Firebat::Firebat(Firebat* firebat, bool _is_enemy) : Unit(firebat, _is_enemy)
{
	p = firebat->p;
	p->current_animation = &p->up;
	p->up.speed = firebat->p->anim_speed;
	p->down.speed = firebat->p->anim_speed;
	p->down_left.speed = firebat->p->anim_speed;
	p->down_right.speed = firebat->p->anim_speed;
	p->left.speed = firebat->p->anim_speed;
	p->right.speed = firebat->p->anim_speed;
	p->up_left.speed = firebat->p->anim_speed;
	p->up_right.speed = firebat->p->anim_speed;
}


Firebat::~Firebat()
{
	p = NULL;
}

void Firebat::Draw()
{
	Unit::Draw();
	if (state == UNIT_ATTACK)
		App->render->Blit(&p->sprite, true);
}

void Firebat::SetAnimation()
{
	Unit::SetAnimation();
	if (state == UNIT_ATTACK)
	{
		if (current_animation == &a_right)
		{
			p->current_animation = &p->right;
			p->current_pos = p->pos_right;
		}

		else if (current_animation == &a_down_right)
		{
			p->current_animation = &p->down_right;
			p->current_pos = p->pos_down_right;
		}

		else if (current_animation == &a_down)
		{
			p->current_animation = &p->down;
			p->current_pos = p->pos_down;
		}

		else if (current_animation == &a_left)
		{
			p->current_animation = &p->left;
			p->current_pos = p->pos_left;
		}

		else if (current_animation == &a_down_left)
		{
			p->current_animation = &p->down_left;
			p->current_pos = p->pos_down_left;
		}

		else if (current_animation == &a_up_left)
		{
			p->current_animation = &p->up_left;
			p->current_pos = p->pos_up_left;
		}

		else if (current_animation == &a_up)
		{
			p->current_animation = &p->up;
			p->current_pos = p->pos_up;
		}

		else if (current_animation == &a_up_right)
		{
			p->current_animation = &p->up_right;
			p->current_pos = p->pos_up_right;
		}

		p->sprite.position.x = GetDrawPosition().x + p->current_pos.x;
		p->sprite.position.y = GetDrawPosition().y + p->current_pos.y;

		p->sprite.rect.x = p->current_animation->getCurrentFrame(App->entity->bullet_time).x;
		p->sprite.rect.y = p->current_animation->getCurrentFrame(App->entity->bullet_time).y;
	}
}