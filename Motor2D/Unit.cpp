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
	App->render->Blit(texture, pos.x, pos.y, NULL);
}