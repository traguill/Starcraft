#ifndef __MARINE_H__
#define __MARINE_H__

#include "Unit.h"

class Marine : public Unit
{
public:
	Marine();
	Marine(Unit* unit);
	Marine(Marine* marine, bool _is_enemy);

	~Marine();
};

#endif