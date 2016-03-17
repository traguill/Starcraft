

#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "p2Defs.h"
#include "Entity.h"


enum TYPE{CENTER,FACTORY,TURRET,BUNKER,TOWER};


class Building : public Entity
{
public:
	bool Update(float dt);
	void Draw();

private:
	TYPE type;
};

#endif