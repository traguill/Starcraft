#ifndef __UIBUTTON_H__
#define __UIBUTTON_H__

#include "UIEntity.h"
#include "UIImage.h"
#include "UILabel.h"

struct SDL_Texture;

enum BUTTON_STATE
{
	IDLE,
	PRESSED,
	HOVER
};

class UIButton : public UIEntity
{
public:

	UIButton();

	UIButton(const char* _text, const int x, const int y, SDL_Rect section_idle, SDL_Rect section_pressed, SDL_Rect section_hover);

	// Destructor
	virtual ~UIButton();

	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

private:

	void GetState(); //Sets the state of the button (idle, pressed, hover)

private:
	SDL_Rect idle;
	SDL_Rect pressed;
	SDL_Rect hover;
public:
	UILabel		text;
	BUTTON_STATE state;
};

#endif