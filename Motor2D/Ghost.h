#ifndef __GHOST_H__
#define __GHOST_H__

#include "Unit.h"

class Ghost : public Unit
{
public:
	Ghost();
	Ghost(Unit* unit);
	Ghost(Ghost* ghost, bool _is_enemy);

	~Ghost();

	void Update(float dt);

private:
	bool snipping = false;
	bool has_hit = false;

public:
	bool GetSnipping() const;
	bool HasHit() const;
	void BulletHits();
	
	void DisableSnipper();
	void Shoot(int x, int y);

private:
	void Attack(float dt);
	void CastAbility(const UNIT_ABILITY ability);

	void Invisibility();
	void SetVisible();

	void Snipper();
	
};

#endif