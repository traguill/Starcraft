#ifndef __BULLET_H__
#define __BULLET_H__

#include "Ghost.h"

#define HIT_RADIUS 50

using namespace std;

class Bullet : public Entity
{
	friend class j1EntityManager;	//Provisional
public:

	Bullet();

	Bullet(Bullet* b);

	// Bullet
	 ~Bullet();

	void Update(float dt);

	void Draw();

	void Destroy();

public:

	fPoint direction;


	iPoint destination;

	Ghost* source;
private:
	float speed = 900;

	iPoint pos_up;
	iPoint pos_down;
	iPoint pos_right;
	iPoint pos_left;
	iPoint pos_up_right;
	iPoint pos_down_right;
	iPoint pos_up_left;
	iPoint pos_down_left;

	fPoint float_pos;
	bool bullet_started = false;
};

#endif