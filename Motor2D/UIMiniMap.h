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

	UIMiniMap(SDL_Rect position, SDL_Rect section_drawn, iPoint original_map_size);

	// Destructor
	virtual ~UIMiniMap();

	bool PreUpdate();
	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

	void EnableMinimap(bool enable);

	void GetScreenPos(int &x, int &y)const;
	void GetLocalPos(int &x, int &y)const;
	SDL_Rect GetScreenRect()const;
	SDL_Rect GetLocalRect()const;
	void SetLocalPos(int x, int y);

private:

	void GetState();
	iPoint WhiteRectUpdatedPos();

	void UpdateRect();
	void UpdateUnitsMiniMap();

private:

	bool enable_minimap = true;

	MAP_STATE map_state;

	//To get scale
	int div_x;
	int div_y;

	//Map offset
	iPoint offset;

	//Position where the white rect is drawn
	iPoint draw_pos;

	MAP_STATE last_state;

	Sprite ui_sprite;
	Sprite white_rect;
};

#endif