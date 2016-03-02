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

	UIButton(const char* _text, const int x, const int y, const char* path_idle = NULL, const char* path_pressed = NULL, const char* path_hover = NULL);

	// Destructor
	virtual ~UIButton();

	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

private:

	void GetState(); //Sets the state of the button (idle, pressed, hover)

private:
	SDL_Texture* idle;
	SDL_Texture* pressed;
	SDL_Texture* hover;
public:
	UILabel		text;
	BUTTON_STATE state;
};

#endif