#include "Bullet.h"
#include "j1Render.h"
#include "j1App.h"
#include "EntityManager.h"
#include "j1Textures.h"

Bullet::Bullet()
{
	bullet_started = true;
	float_pos.x = logic_pos.x;
	float_pos.y = logic_pos.y;
}

Bullet::Bullet(Bullet* b)
{
	bullet_started = true;
	sprite.texture = b->sprite.texture;
	sprite.rect.w = b->sprite.rect.w;
	sprite.rect.h = b->sprite.rect.h;

	pos_up = b->pos_up;
	pos_down = b->pos_down;
	pos_right = b->pos_right;
	pos_left = b->pos_left;
	pos_up_right = b->pos_up_right;
	pos_up_right_1 = b->pos_up_right_1;
	pos_up_right_2 = b->pos_up_right_2;
	pos_down_right = b->pos_down_right;
	pos_down_right_1 = b->pos_down_right_1;
	pos_down_right_2 = b->pos_down_right_2;
	pos_up_left	= b->pos_up_left;
	pos_up_left_1 = b->pos_up_left_1;
	pos_up_left_2 = b->pos_up_left_2;
	pos_down_left =b->pos_down_left;
	pos_down_left_1 = b->pos_down_left_1;
	pos_down_left_2 = b->pos_down_left_2;
	
}

// Destructor
Bullet::~Bullet()
{
	source = NULL;
}


void Bullet::Update(float dt)
{
	//fPoint float_pos; float_pos.x = logic_pos.x, float_pos.y = logic_pos.y;
	if (bullet_started == true)
	{
		float_pos.x = logic_pos.x;
		float_pos.y = logic_pos.y;
		bullet_started = false;
	}

	float_pos.x += (direction.x * speed) *dt;
	float_pos.y += (direction.y * speed) *dt;

	//float_pos.x = roundf(float_pos.x);
	//float_pos.y = roundf(float_pos.y);

	logic_pos.x = roundf(float_pos.x);
	logic_pos.y = roundf(float_pos.y);

	//Check if hits an enemy
	list<Unit*>::iterator enemy = App->entity->enemy_units.begin();

	while (enemy != App->entity->enemy_units.end())
	{
		SDL_Rect collider;
		collider.x = (*enemy)->GetDrawPosition().x;
		collider.y = (*enemy)->GetDrawPosition().y;
		collider.w = (*enemy)->width;
		collider.h = (*enemy)->height;
		if (logic_pos.x >= collider.x && logic_pos.x <= collider.x + collider.w && logic_pos.y >= collider.y && logic_pos.y <= collider.y + collider.h)
		{
			if ((*enemy)->state != UNIT_DIE)
			{
				(*enemy)->ApplyDamage(1000, source);
			}
			
		}
		++enemy;
	}

	if (logic_pos.DistanceTo(destination) <= HIT_RADIUS)
	{
		Destroy();
	}

	//Update bullet animation
	//iPoint dir(roundf(direction.x), roundf(direction.y));
	
	float angle;

	if (direction.x == 0)
	{
		if (-direction.y < 0)
			angle = 90;
		if (-direction.y > 0)
			angle = 270;
	}
	else
	{
		fPoint direc = direction;
		direc.y = -direction.y;
		angle = RADTODEG*(direc.AngleFromVector());

	}
	
	//Up
	if (angle <= (90 + 45/4) && angle >= (90 - 45/4))
	{
		sprite.rect.x = pos_up.x;
		sprite.rect.y = pos_up.y;
	}

	//Up-Right
	if (angle <= (45 + 45 / 4) && angle >= (45 - 45 / 4))
	{
		sprite.rect.x = pos_up_right.x;
		sprite.rect.y = pos_up_right.y;
	}

	//Up-Right 1
	if (angle < (90 - 45 / 4) && angle > (45 + 45 / 4))
	{
		sprite.rect.x = pos_up_right_1.x;
		sprite.rect.y = pos_up_right_1.y;
	}

	//Up-Right 2
	if (angle < (45 - 45 / 4) && angle >(45 / 4))
	{
		sprite.rect.x = pos_up_right_2.x;
		sprite.rect.y = pos_up_right_2.y;
	}

	//Right
	if (angle <= (45 / 4) && angle >= (- 45 / 4))
	{
		sprite.rect.x = pos_right.x;
		sprite.rect.y = pos_right.y;
	}

	//Down-right
	if (angle >= -(45 + 45 / 4) && angle <= -(45 - 45 / 4))
	{
		sprite.rect.x = pos_down_right.x;
		sprite.rect.y = pos_down_right.y;
	}

	//Down-right 1
	if (angle > -(90 - 45 / 4) && angle < -(45 + 45 / 4))
	{
		sprite.rect.x = pos_down_right_1.x;
		sprite.rect.y = pos_down_right_1.y;
	}

	//Down-right 2
	if (angle > -(45 - 45 / 4) && angle < -(45 / 4))
	{
		sprite.rect.x = pos_down_right_2.x;
		sprite.rect.y = pos_down_right_2.y;
	}

	//Down
	if (angle >= -(90 + 45 / 4) && angle <= -(90 - 45 / 4))
	{
		sprite.rect.x = pos_down.x;
		sprite.rect.y = pos_down.y;
	}

	//Down-left
	if (angle >= -(135 + 45 / 4) && angle <= -(135 - 45 / 4))
	{
		sprite.rect.x = pos_down_left.x;
		sprite.rect.y = pos_down_left.y;
	}

	//Down-left 1
	if (angle < -(135 + 45 / 4) && angle > -(90 + 45 / 4))
	{
		sprite.rect.x = pos_down_left_1.x;
		sprite.rect.y = pos_down_left_1.y;
	}

	//Down-left 2
	if (angle > -(180 - 45 / 4) && angle < -(135 + 45 / 4))
	{
		sprite.rect.x = pos_down_left_2.x;
		sprite.rect.y = pos_down_left_2.y;
	}

	//Left
	if (angle <= (180 + 45 / 4) && angle >= (180 - 45 / 4))
	{
		sprite.rect.x = pos_left.x;
		sprite.rect.y = pos_left.y;
	}

	//Up-Left
	if (angle > (135 - 45 / 4) && angle < (135 + 45 / 4))
	{
		sprite.rect.x = pos_up_left.x;
		sprite.rect.y = pos_up_left.y;
	}

	//Up-Left 1
	if (angle > (90 + 45 / 4) && angle < (135 - 45 / 4))
	{
		sprite.rect.x = pos_up_left_1.x;
		sprite.rect.y = pos_up_left_1.y;
	}

	//Up-Left 2
	if (angle > (135 + 45 / 4) && angle < (180 - 45 / 4))
	{
		sprite.rect.x = pos_up_left_2.x;
		sprite.rect.y = pos_up_left_2.y;
	}

	
}

void Bullet::Draw()
{
	//App->render->DrawQuad({ logic_pos.x, logic_pos.y, 10, 10 }, 0, 255, 0, 255, true, true);

	sprite.position.x = logic_pos.x - (sprite.rect.w / 2);
	sprite.position.y = logic_pos.y - (sprite.rect.h / 2);
	App->render->Blit(&sprite);
}


void Bullet::Destroy()
{
	if (source != NULL)
	source->BulletHits();
	App->entity->RemoveBullet(this);
}