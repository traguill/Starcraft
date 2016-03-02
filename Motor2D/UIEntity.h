#ifndef __UIENTITY_H__
#define __UIENTITY_H__

#include "p2Point.h"
#include "SDL/include/SDL.h"
#include <list>

using namespace std;

class j1Module;

enum GUI_TYPE
{
	LABEL,
	IMAGE,
	BUTTON,
	INPUT_BOX
};

enum GUI_EVENTS
{
	NONE,
	MOUSE_ENTER,
	MOUSE_EXIT,
	MOUSE_BUTTON_RIGHT_DOWN,
	MOUSE_BUTTON_RIGHT_UP
};

class UIEntity
{
public:

	UIEntity();

	// Destructor
	virtual ~UIEntity();

	virtual bool Update(float dt);
	// Called before quitting

	virtual bool CleanUp();

	void GUIEvents();

	void GetScreenPos(int &x, int &y)const;
	void GetLocalPos(int &x, int &y)const;
	SDL_Rect GetScreenRect()const;
	SDL_Rect GetLocalRect()const;
	void SetLocalPos(int x, int y);

	void SetParent(UIEntity* _parent);
	
	void Debug();
	void Drag();
	//Hides or shows an UI element
	void SetVisible(bool visible);
	bool IsVisible()const;

public:

	GUI_TYPE			type;
	GUI_EVENTS			gui_event;
	j1Module*			listener;
	list<UIEntity*>	childs;
	bool				interactable = false;
	bool				focusable = true;
	bool				isFocus = false;
	
protected:
	SDL_Rect rect; //Local

private:
	bool				isVisible = true;
	UIEntity*			parent = NULL;

};

#endif