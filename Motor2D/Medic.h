#ifndef __MEDIC_H__
#define __MEDIC_H__

#include "Unit.h"

class Medic : public Unit
{
public:
	Medic();
	Medic(Unit* unit);
	Medic(Medic* medic, bool _is_enemy);

	~Medic();
};

#endif