#ifndef __UICURSOR_H__
#define __UICURSOR_H__

#include "UIEntity.h"
#include "UIImage.h"
//UIMASTER
struct SDL_Texture;

class UICursor : public UIEntity
{
public:

	UICursor();

	UICursor(vector<SDL_Rect> sections, float anim_speed);

	// Destructor
	virtual ~UICursor();

	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

private:

	void GetState(); //Sets the state of the button (idle, pressed, hover)

public:
	Animation anim;
	Animation down_cursor;
	Animation up_cursor;
	Animation right_cursor;
	Animation left_cursor; // no hi ha al atlas
	Animation friendly_sel;
};

#endif