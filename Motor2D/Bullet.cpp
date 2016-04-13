#include "Bullet.h"
#include "j1Render.h"
#include "j1App.h"
#include "EntityManager.h"
#include "j1Textures.h"

Bullet::Bullet()
{}

// Destructor
Bullet::~Bullet()
{
	source = NULL;
}


void Bullet::Update(float dt)
{
	logic_pos.x += direction.x * speed * dt;
	logic_pos.y += direction.y * speed * dt;

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
			(*enemy)->ApplyDamage(1000, source);
			Destroy();
			break;
		}
		++enemy;
	}

	if (logic_pos.DistanceTo(destination) <= HIT_RADIUS)
	{
		Destroy();
	}
}

void Bullet::Draw()
{
	App->render->DrawQuad({ logic_pos.x, logic_pos.y, 10, 10 }, 0, 255, 0, 255, true, true);
	//App->render->Blit(&sprite);
}


void Bullet::Destroy()
{
	if (source != NULL)
	source->has_hit = true;
	App->entity->bullets.remove(this);
	delete this;
}