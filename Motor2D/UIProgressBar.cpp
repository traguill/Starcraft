#include "UIProgressBar.h"
#include "j1App.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "j1UIManager.h"
#include "p2Log.h"

UIProgressBar::UIProgressBar() : UIEntity()
{
	type = PROGRESS_BAR;
	hp_state = FULL;

	interactable = false;
}

UIProgressBar::UIProgressBar(UIProgressBar* bar) : UIEntity()
{
	full_bar_section = bar->full_bar_section;
	empty_bar_section = bar->empty_bar_section;
	low_bar_section = bar->low_bar_section;
	middle_bar_section = bar->middle_bar_section;

	max_number = bar->max_number;

	bar_tex = bar->bar_tex;
	
	current_number = max_number;

	hp_state = FULL;
	bar_type = bar->bar_type;

	ui_sprite.texture = bar_tex;
	ui_sprite.rect = full_bar_section;
}

// Destructor
UIProgressBar::~UIProgressBar()
{
}
bool UIProgressBar::Update(float dt)
{
	bool ret = true;
	

	return ret;
}

// Called before quitting
bool UIProgressBar::CleanUp(){

	bool ret = true;

	App->tex->UnLoad(bar_tex);
	ui_sprite.texture = NULL;
	bar_tex = NULL;

	return ret;
}

void UIProgressBar::SetValue(int value)
{
	current_number = value;

	if (bar_type == HEALTH)
	{
		if (current_number < 0)
			hp_state = EMPTY;

		if (current_number > 0 && current_number <= max_number / 3)
			hp_state = LOW;

		if (current_number > max_number / 3 && current_number <= (2 * max_number) / 3)
			hp_state = MIDDLE;

		if (current_number > (2 * max_number) / 3)
			hp_state = FULL;
	}
}

void UIProgressBar::Draw(int x, int y)
{
	ui_sprite.position.x = x + rect.x;
	ui_sprite.position.y = y + rect.y;
	ui_sprite.rect = empty_bar_section;
	App->render->BlitUI(ui_sprite);

	SDL_Rect section = empty_bar_section;

	float width = ((float)current_number / (float)max_number) * (float)empty_bar_section.w;
	section.w = width;


	switch (hp_state)
	{
	case EMPTY:
		section.x = empty_bar_section.x;
		section.y = empty_bar_section.y;
		break;
	case LOW:
		section.x = low_bar_section.x;
		section.y = low_bar_section.y;
		break;
	case MIDDLE:
		section.x = middle_bar_section.x;
		section.y = middle_bar_section.y;
		break;
	case FULL:
		section.x = full_bar_section.x;
		section.y = full_bar_section.y;
		break;
	}

	ui_sprite.position.x = x + rect.x;
	ui_sprite.position.y = y + rect.y;
	ui_sprite.rect = section;

	App->render->BlitUI(ui_sprite);
}


Sprite* UIProgressBar::GetSprite()
{
	return &ui_sprite;
}