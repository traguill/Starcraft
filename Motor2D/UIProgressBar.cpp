#include "UIProgressBar.h"
#include "j1App.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "j1UIManager.h"

UIProgressBar::UIProgressBar() : UIEntity()
{
	type = PROGRESS_BAR;
	hp_state = FULL;

	interactable = true;
}

UIProgressBar::UIProgressBar(BAR_TYPE _type, int max_num, const int x, const int y, const int w, const int h, SDL_Rect full_bar, SDL_Rect empty_bar, SDL_Rect low_bar, SDL_Rect middle_bar, SDL_Texture* texture) : UIEntity ()
{
	type = PROGRESS_BAR;
	bar_ty = _type;

	if (bar_ty == HEALTH){
		hp_state = FULL;
	}

	else
		hp_state = EMPTY;

	max_number = max_num;

	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

	full_bar_section = full_bar;
	empty_bar_section = empty_bar;
	low_bar_section = low_bar;
	middle_bar_section = middle_bar;

	bar_tex = texture;

	interactable = true;

}


UIProgressBar::UIProgressBar(BAR_TYPE _type, const int x, const int y, const int w, const int h, SDL_Rect full_bar, SDL_Rect empty_bar, SDL_Rect low_bar, SDL_Rect middle_bar, SDL_Texture* texture) : UIEntity()
{
	type = PROGRESS_BAR;
	bar_ty = _type;

	if (bar_ty == HEALTH){
		hp_state = FULL;
	}

	else
		hp_state = EMPTY;

	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

	full_bar_section = full_bar;
	empty_bar_section = empty_bar;
	low_bar_section = low_bar;
	middle_bar_section = middle_bar;

	bar_tex = texture;

	interactable = true;

}

UIProgressBar::UIProgressBar(BAR_TYPE _type, int max_num, const int x, const int y, UIProgressBar* prog_bar) : UIEntity()
{
	type = PROGRESS_BAR;
	bar_ty = _type;
	
	if (bar_ty == HEALTH){
		hp_state = FULL;
	}
	
	else
		hp_state = EMPTY;

	max_number = max_num;
	
	rect.x = x + prog_bar->rect.x;
	rect.y = y + prog_bar->rect.y;
	rect.w = prog_bar->rect.w;
	rect.h = prog_bar->rect.h;

	full_bar_section = prog_bar->full_bar_section;
	empty_bar_section = prog_bar->empty_bar_section;
	low_bar_section = prog_bar->low_bar_section;
	middle_bar_section = prog_bar->middle_bar_section;

	bar_tex = prog_bar->bar_tex;

	interactable = true;
}

// Destructor
UIProgressBar::~UIProgressBar()
{
	CleanUp();
}
bool UIProgressBar::Update(float dt)
{
	bool ret = true;

	//int x, y;
	//GetScreenPos(x, y);
	
		if (current_number <= 0)
		{
			hp_state = EMPTY;
		}

		if (current_number > 0 && current_number < max_number / 3)
		{
			hp_state = LOW;
		}

		if (current_number >= (max_number / 3) && current_number < (max_number * 2 / 3))
		{
			hp_state = MIDDLE;
		}
		if (current_number >= (max_number * 2 / 3))
		{
			hp_state = FULL;
		}

	return ret;
}


int UIProgressBar::GetMaxSize() const
{
	return max_number;
}

void UIProgressBar::SetMaxNum(const int max_num)
{
	max_number = max_num;
}

void UIProgressBar::SetBarsLength(int _width)
{
	low_bar_section.w = _width;
	full_bar_section.w = _width;
	middle_bar_section.w = _width;

}


SDL_Rect UIProgressBar::GetFullBar() const
{

	return full_bar_section;
}

SDL_Rect UIProgressBar::GetEmptyBar() const
{

	return empty_bar_section;
}

SDL_Rect UIProgressBar::GetLowBar() const
{

	return low_bar_section;
}

SDL_Rect UIProgressBar::GetMiddleBar() const
{

	return middle_bar_section;
}

SDL_Texture* UIProgressBar::GetTexture() const
{
	return bar_tex;
}

// Called before quitting
bool UIProgressBar::CleanUp(){

	bool ret = true;

	bar_tex = NULL;

	return ret;
}
