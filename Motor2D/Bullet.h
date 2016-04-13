#ifndef __BULLET_H__
#define __BULLET_H__

#include "Entity.h"
#include "Unit.h"

#define HIT_RADIUS 50

using namespace std;

class Bullet : public Entity
{
	friend class j1EntityManager;	//Provisional
public:

	Bullet();

	// Bullet
	 ~Bullet();

	void Update(float dt);

	void Draw();

	void Destroy();

public:

	fPoint direction;


	iPoint destination;

	Unit* source;
private:
	float speed = 100;
};

#endif