#include "Marine.h"

Marine::Marine() : Unit()
{

}

Marine::Marine(Unit* unit) : Unit(unit, false)
{
	if (is_enemy == false)
	{
		friendly_life = unit->friendly_life;
		friendly_damage = unit->friendly_damage;
	}

	auxiliar_texture = unit->GetAuxiliarTexture();
	walk_anim_speed = unit->walk_anim_speed;
	idle_anim_speed = unit->idle_anim_speed;
	attack_anim_speed = unit->attack_anim_speed;
	death_anim_speed = unit->death_anim_speed;
}

Marine::Marine(Marine* marine, bool _is_enemy) : Unit(marine, _is_enemy)
{
	if (is_enemy == false)
	{
		life = marine->friendly_life;
		damage = marine->friendly_damage;
		max_life = life;
	}
}

Marine::~Marine()
{

}