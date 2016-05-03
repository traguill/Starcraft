#include "Unit.h"
#include "Medic.h"

Medic::Medic() : Unit()
{

}

Medic::Medic(Unit* unit) : Unit(unit, false)
{
	auxiliar_texture = unit->GetAuxiliarTexture();
	walk_anim_speed = unit->walk_anim_speed;
	idle_anim_speed = unit->idle_anim_speed;
	attack_anim_speed = unit->attack_anim_speed;
	death_anim_speed = unit->death_anim_speed;
}

Medic::Medic(Medic* medic, bool _is_enemy) : Unit(medic, _is_enemy)
{

}

Medic::~Medic()
{

}