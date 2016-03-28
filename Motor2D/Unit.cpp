#include "Unit.h"
#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"




void Unit::Update(float dt)
{
	
}

void Unit::Draw()
{
	SDL_Rect r;
	r.x = pos.x;
	r.y = pos.y;
	r.w = 24;
	r.h = 32;
	if (selected == true)
		App->render->DrawQuad(r, 0, 255, 0, 255, false, true);

	App->render->Blit(texture, pos.x, pos.y, NULL);
}