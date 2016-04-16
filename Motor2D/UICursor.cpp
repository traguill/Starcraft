#include "UICursor.h"
#include "j1App.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "UILabel.h"
#include "j1UIManager.h"
#include "j1Input.h"
#include "EntityManager.h"
#include "SceneManager.h"

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
	friendly_sel.frames.clear();
	enemy_sel.frames.clear();

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

	section.x = 267; section.y = 62; section.w = 36; section.h = 21;
	up_cursor.frames.push_back(section);
	section.x = 304; section.y = 62; section.w = 36; section.h = 21;
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

	section.x = 2; section.y = 94, section.w = 41, section.h = 43;
	friendly_sel.frames.push_back(section);
	section.x = 46; section.y = 94, section.w = 41, section.h = 43;
	friendly_sel.frames.push_back(section);
	section.x = 90; section.y = 94, section.w = 41, section.h = 43;
	friendly_sel.frames.push_back(section);
	section.x = 134; section.y = 94, section.w = 41, section.h = 43;
	friendly_sel.frames.push_back(section);
	section.x = 178; section.y = 94, section.w = 41, section.h = 43;
	friendly_sel.frames.push_back(section);
	section.x = 222; section.y = 94, section.w = 41, section.h = 43;
	friendly_sel.frames.push_back(section);
	section.x = 266; section.y = 94, section.w = 41, section.h = 43;
	friendly_sel.frames.push_back(section);
	section.x = 310; section.y = 94, section.w = 41, section.h = 43;
	friendly_sel.frames.push_back(section);
	section.x = 354; section.y = 94, section.w = 41, section.h = 43;
	friendly_sel.frames.push_back(section);
	section.x = 398; section.y = 94, section.w = 41, section.h = 43;
	friendly_sel.frames.push_back(section);
	section.x = 442; section.y = 94, section.w = 41, section.h = 43;
	friendly_sel.frames.push_back(section);
	section.x = 486; section.y = 94, section.w = 41, section.h = 43;
	friendly_sel.frames.push_back(section);
	section.x = 530; section.y = 94, section.w = 41, section.h = 43;
	friendly_sel.frames.push_back(section);
	section.x = 574; section.y = 94, section.w = 41, section.h = 43;
	friendly_sel.frames.push_back(section);
	friendly_sel.loop = true;
	friendly_sel.speed = 0.08;

	section.x = 2; section.y = 186, section.w = 41, section.h = 43;
	enemy_sel.frames.push_back(section);
	section.x = 46; section.y = 186, section.w = 41, section.h = 43;
	enemy_sel.frames.push_back(section);
	section.x = 90; section.y = 186, section.w = 41, section.h = 43;
	enemy_sel.frames.push_back(section);
	section.x = 134; section.y = 186, section.w = 41, section.h = 43;
	enemy_sel.frames.push_back(section);
	section.x = 178; section.y = 186, section.w = 41, section.h = 43;
	enemy_sel.frames.push_back(section);
	section.x = 222; section.y = 186, section.w = 41, section.h = 43;
	enemy_sel.frames.push_back(section);
	section.x = 266; section.y = 186, section.w = 41, section.h = 43;
	enemy_sel.frames.push_back(section);
	section.x = 310; section.y = 186, section.w = 41, section.h = 43;
	enemy_sel.frames.push_back(section);
	section.x = 354; section.y = 186, section.w = 41, section.h = 43;
	enemy_sel.frames.push_back(section);
	section.x = 398; section.y = 186, section.w = 41, section.h = 43;
	enemy_sel.frames.push_back(section);
	section.x = 442; section.y = 186, section.w = 41, section.h = 43;
	enemy_sel.frames.push_back(section);
	section.x = 486; section.y = 186, section.w = 41, section.h = 43;
	enemy_sel.frames.push_back(section);
	section.x = 530; section.y = 186, section.w = 41, section.h = 43;
	enemy_sel.frames.push_back(section);
	section.x = 574; section.y = 186, section.w = 41, section.h = 43;
	enemy_sel.frames.push_back(section);
	enemy_sel.loop = true;
	enemy_sel.speed = 0.08;
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

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT && App->scene_manager->in_game == true)
	{
		//MARTI: dont' use operate new inside Blit method Blit(new SDL_Rect{x,y,w,h}) 
		//DO NOT use magic numbers pls...
		SDL_Rect section{ 310, 48, 41, 43 };
		App->render->Blit(App->entity->gui_cursor, rect.x - 20, rect.y - 19, &section);
	}

	else
	{
		if (App->ui->cursor_state == TO_RIGHT)
		{
			App->render->Blit(App->ui->GetAtlas(), MIN(rect.x, App->render->camera.w - App->render->camera.x - 21), rect.y, &right_cursor.getCurrentFrame());
		}

		if (App->ui->cursor_state == TO_LEFT)
		{
			App->render->Blit(App->ui->GetAtlas(), rect.x, rect.y, &left_cursor.getCurrentFrame());
		}

		if (App->ui->cursor_state == UP)
		{
			App->render->Blit(App->ui->GetAtlas(), rect.x, rect.y, &up_cursor.getCurrentFrame());
		}

		if (App->ui->cursor_state == DOWN)
		{
			App->render->Blit(App->ui->GetAtlas(), rect.x, MIN(rect.y, App->render->camera.h - App->render->camera.y - 21), &down_cursor.getCurrentFrame());
		}

		if (App->ui->cursor_state == ON_FRIENDLY)
		{
			App->render->Blit(App->entity->gui_cursor, rect.x - 21, rect.y - 21, &friendly_sel.getCurrentFrame());
		}

		if (App->ui->cursor_state == ON_ENEMY)
		{
			App->render->Blit(App->entity->gui_cursor, rect.x - 21, rect.y - 21, &enemy_sel.getCurrentFrame());
		}

		if (App->ui->cursor_state == STANDARD)
		{
			App->render->Blit(App->ui->GetAtlas(), rect.x, rect.y, &anim.getCurrentFrame());
		}
	}

	return ret;
}

bool UICursor::CleanUp()
{
	bool ret = true;

	return ret;
}