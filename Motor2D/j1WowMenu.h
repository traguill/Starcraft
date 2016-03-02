#ifndef __J1WOWMENU_H__
#define __J1WOWMENU_H__

#include "j1Module.h"

struct SDL_Texture;
class UILabel;
class UIImage;
class UIButton;
class UIInputBox;

class j1WowMenu : public j1Module
{
public:

	j1WowMenu();

	// Destructor
	virtual ~j1WowMenu();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	void OnGUI(UIEntity* gui, GUI_EVENTS event);

private:

	UILabel*		label_test;
	UIImage*		window;
	UIButton*		login;
	UIImage*		background;
	UIInputBox*		password_box;

};

#endif // __j1WowMenu_H__