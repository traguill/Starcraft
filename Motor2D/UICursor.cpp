#include "UICursor.h"
#include "j1App.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "UILabel.h"
#include "j1UIManager.h"
#include "j1Input.h"

UICursor::UICursor() : UIEntity()
{
	type = CURSOR;
}

UICursor::UICursor(vector<SDL_Rect> sections, float anim_speed) : UIEntity()
{
	type = CURSOR;

	App->input->GetMousePosition(rect.x, rect.y);

	anim.frames.clear();

	//adding anim frames
	for (std::vector<SDL_Rect>::iterator it = sections.begin(); it != sections.end(); ++it)
		anim.frames.push_back(*it);

	anim.speed = anim_speed;
	anim.loop = true;
}

// Destructor
UICursor::~UICursor()
{}

// Called before render is available
bool UICursor::Update(float dt)
{
	bool ret = true;

	App->input->GetMousePosition(rect.x, rect.y);

	iPoint cam_pos(App->render->camera.x, App->render->camera.y);

	rect.x -= cam_pos.x;
	rect.y -= cam_pos.y;

	App->render->Blit(App->ui->GetAtlas(), rect.x, rect.y, &anim.getCurrentFrame());

	return ret;
}

bool UICursor::CleanUp()
{
	bool ret = true;

	return ret;
}