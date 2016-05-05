#ifndef __UIMINIMAP_H__
#define __UIMINIMAP_H__

#include "UIEntity.h"
#include "UIImage.h"


struct SDL_Texture;

enum MAP_STATE
{
	IDLE_MAP,
	HOVER_MAP,
	PRESSED_MAP,
	CONTINUE_PRESS_MAP
};

class UIMiniMap : public UIEntity
{
public:

	UIMiniMap();

	UIMiniMap(SDL_Rect position, SDL_Rect section_drawn);

	// Destructor
	virtual ~UIMiniMap();

	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

private:

	void GetState();
	iPoint WhiteRectUpdatedPos();

	void UpdateRect();
	void UpdateUnitsMiniMap();

private:
	SDL_Rect draw_section;
	SDL_Rect white_rec;

	MAP_STATE map_state;

	iPoint init_pos;

	//To get scale
	int div_x;
	int div_y;

	//Map offset
	iPoint offset;

	//Position where the white rect is drawn
	iPoint draw_pos;

	bool click_transition = false;
};

#endif