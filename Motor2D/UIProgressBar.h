#ifndef __UI_PROGRESS_BAR_H__
#define __UI_PROGRESS_BAR_H__

#include "UIEntity.h"

struct SDL_Texture;

enum BAR_TYPE
{
	HEALTH,
	MANA
};
enum BAR_STATE
{
	EMPTY,
	LOW,
	MIDDLE,
	FULL
};

class UIProgressBar : public UIEntity

{

public:

	UIProgressBar();

	UIProgressBar(UIProgressBar* bar);


	// Destructor
	virtual ~UIProgressBar();

	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

	void SetValue(int value);
	void Draw(int x, int y);

	Sprite* GetSprite();

private:

	Sprite ui_sprite;


public:
	SDL_Texture* bar_tex;
	int current_number;
	BAR_STATE hp_state;
	BAR_TYPE bar_type;

	SDL_Rect full_bar_section;
	SDL_Rect empty_bar_section;
	SDL_Rect low_bar_section;
	SDL_Rect middle_bar_section;

	int max_number;
};

#endif