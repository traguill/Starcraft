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
	down_cursor.frames.clear();
	up_cursor.frames.clear();
	right_cursor.frames.clear();
	left_cursor.frames.clear();

	//adding anim frames
	for (std::vector<SDL_Rect>::iterator it = sections.begin(); it != sections.end(); ++it)
		anim.frames.push_back(*it);

	anim.speed = anim_speed;
	anim.loop = true;

	SDL_Rect section;

	section.x = 106; section.y = 62; section.w = 36, section.h = 21;
	down_cursor.frames.push_back(section);
	section.x = 143; section.y = 62; section.w = 36, section.h = 21;
	down_cursor.frames.push_back(section);
	down_cursor.loop = true;
	down_cursor.speed = 0.08;

	section.x = 106; section.y = 62; section.w = 36; section.h = 21;
	up_cursor.frames.push_back(section);
	section.x = 143; section.y = 62; section.w = 36; section.h = 21;
	up_cursor.frames.push_back(section);
	up_cursor.loop = true;
	up_cursor.speed = 0.08;

	section.x = 225; section.y = 63, section.w = 20; section.h = 34;
	right_cursor.frames.push_back(section);
	section.x = 246; section.y = 63, section.w = 20; section.h = 34;
	right_cursor.frames.push_back(section);
	right_cursor.loop = true;
	right_cursor.speed = 0.08;

	section.x = 180; section.y = 63, section.w = 20, section.h = 34;	// no hi ha al atlas up
	left_cursor.frames.push_back(section);
	section.x = 202; section.y = 63, section.w = 20, section.h = 34;
	left_cursor.frames.push_back(section);
	left_cursor.loop = true;
	left_cursor.speed = 0.08;
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

	if (App->ui->cursor_state == to_right)
	{
		App->render->Blit(App->ui->GetAtlas(), rect.x, rect.y, &right_cursor.getCurrentFrame());
	}

	if (App->ui->cursor_state == to_left)
	{
		App->render->Blit(App->ui->GetAtlas(), rect.x, rect.y, &left_cursor.getCurrentFrame());
	}

	if (App->ui->cursor_state == up)
	{
		App->render->Blit(App->ui->GetAtlas(), rect.x, rect.y, &up_cursor.getCurrentFrame());
	}

	if (App->ui->cursor_state == down)
	{
		App->render->Blit(App->ui->GetAtlas(), rect.x, rect.y, &down_cursor.getCurrentFrame());
	}

	if (App->ui->cursor_state == standart)
	{
		App->render->Blit(App->ui->GetAtlas(), rect.x, rect.y, &anim.getCurrentFrame());
	}

	return ret;
}

bool UICursor::CleanUp()
{
	bool ret = true;

	return ret;
}