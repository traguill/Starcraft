#ifndef __FIREBAT_H__
#define __FIREBAT_H__

#include "Unit.h"
#include "Projectile.h"

class Firebat : public Unit
{
public:
	Firebat();
	Firebat(Unit* unit, Projectile* projectile);
	Firebat(Firebat* firebat, bool _is_enemy);

	~Firebat();

	void Draw();

private:
	Projectile* p;

private:
	void SetAnimation();
};

#endif