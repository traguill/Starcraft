#ifndef __UI_PROGRESS_BAR_H__
#define __UI_PROGRESS_BAR_H__

#include "UIEntity.h"

struct SDL_Texture;

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

	UIProgressBar(int max_num, const int x, const int y, const int w, const int h, SDL_Rect full_bar, SDL_Rect empty_bar, SDL_Rect low_bar, SDL_Rect middle_bar, SDL_Texture* texture);

	// Destructor
	virtual ~UIProgressBar();

	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

public:

	void SetState(BAR_STATE &state);
	
	void SetBarsLength(int new_len);

	int GetMaxSize() const;
	SDL_Rect GetFullBar() const;
	SDL_Rect GetEmptyBar() const;
	SDL_Rect GetLowBar() const;
	SDL_Rect GetMiddleBar() const;
	SDL_Texture* GetTexture() const;

private:

	SDL_Rect full_bar_section;
	SDL_Rect empty_bar_section;
	SDL_Rect low_bar_section;
	SDL_Rect middle_bar_section;

	SDL_Texture* bar_tex;
	int max_number;


public:
	int current_number;
	BAR_STATE hp_state;
};

#endif