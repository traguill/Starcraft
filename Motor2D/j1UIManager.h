#ifndef __J1UIMANAGER_H__
#define __J1UIMANAGER_H__

#include "j1Module.h"


class UILabel;
class UIImage;
class UIEntity;
class UIButton;
class UIInputBox;

// ---------------------------------------------------
class j1UIManager : public j1Module
{
public:

	j1UIManager();

	// Destructor
	virtual ~j1UIManager();

	// Called when before render is available
	bool Awake(pugi::xml_node&);

	// Call before first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	bool Update(float dt);

	// Called after all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// TODO 2: Create the factory methods
	// Gui creation functions

	const SDL_Texture* GetAtlas() const;

	//Creators ------------------------------------------------------------------------------------------------
	UILabel* CreateLabel(const char* text, const int x, const int y, j1Module* listener = NULL);

	UIImage* CreateImage(const char* path, const int x, const int y, j1Module* listener = NULL);
	UIImage* CreateImage(SDL_Texture* tex, const int x, const int y, j1Module* listener = NULL);

	UIButton* CreateButton(const char* _text, const int x, const int y, const char* path_idle = NULL, const char* path_pressed = NULL, const char* path_hover = NULL, j1Module* listener = NULL);

	UIInputBox* CreateInputBox(const char* text, const int x, const int y, const char* path, j1Module* listener = NULL);
	//Functions ---------------------------------------------------------------------------------------------------
	UIEntity* GetMouseHover()const;

private:
	//Utilities ------------------------------------------------------------------------------------------------------
	void GetMouseInput(); //Get input of mouse to drag windows/elements
	void SetNextFocus(); //Sets the focus to the next element

private:

	SDL_Texture* atlas;
	string atlas_file_name;

	list<UIEntity*>		gui_elements;
	UIEntity*				gui_pressed = NULL;
	UIEntity*				focus = NULL;

public:
	bool debug;
};

#endif