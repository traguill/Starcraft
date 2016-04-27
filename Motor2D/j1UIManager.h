#ifndef __J1UIMANAGER_H__
#define __J1UIMANAGER_H__

#include "j1Module.h"
#include "UIProgressBar.h"
#include <map>

class UILabel;
class UIImage;
class UIEntity;
class UIButton;
class UIInputBox;
class UICursor;
class UIProgressBar;
class UIMiniMap;
enum UNIT_TYPE;


enum CURSOR_STATE
{
	STANDARD,
	TO_RIGHT,
	TO_LEFT,
	UP,
	DOWN,
	TO_LEFT_UP,
	TO_LEFT_DOWN,
	TO_RIGHT_UP,
	TO_RIGHT_DOWN,
	DRAG,
	ON_FRIENDLY,
	ON_ENEMY
};

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

	// Gui creation functions

	SDL_Texture* GetAtlas() const;

	//Creators ------------------------------------------------------------------------------------------------
	UILabel* CreateLabel(const char* text, const int x, const int y, bool on_list = true, j1Module* listener = NULL);

	UIImage* CreateImage(SDL_Rect _section, const int x, const int y, bool initial_visible, bool on_list = true,j1Module* listener = NULL);

	UIButton* CreateButton(const char* _text, const int x, const int y, SDL_Rect section_idle, SDL_Rect section_pressed, SDL_Rect section_hover, j1Module* listener = NULL);

	UICursor* CreateCursor(vector<SDL_Rect> sections, float anim_speed, j1Module* listener = NULL);

	UIMiniMap* CreateMiniMap(SDL_Rect _rec, SDL_Rect section_drawn, j1Module* listener = NULL);

	void CreateMiniWireframe(UNIT_TYPE type, uint pos);

	void DeleteMiniWIreframe(uint pos);

	void OcultWireframes();

	//UIInputBox* CreateInputBox(const char* text, const int x, const int y, const char* path, j1Module* listener = NULL);
	//Functions ---------------------------------------------------------------------------------------------------
	UIEntity* GetMouseHover()const;

	void EraseElement(UIEntity* entity);

	void CleanUpList();


private:
	//Utilities ------------------------------------------------------------------------------------------------------
	void GetMouseInput(); //Get input of mouse to drag windows/elements
	void SetNextFocus(); //Sets the focus to the next element
	void ShowIndividualWireframe();
	void ShowMiniWireframes(float dt);
	void RectangleSelection();

	bool LoadUiInfo();

	void DrawLifeMana();

private:

	bool					pressed_last_frame = false;
	SDL_Texture*			atlas;
	string					atlas_file_name;
	string					ui_file_path;

	list<UIEntity*>			gui_elements;
	UIEntity*				gui_pressed = NULL;
	UIEntity*				focus = NULL;

	SDL_Texture*			rects;

	//Snipper ui
	//UIImage*                snipper_ui;
	SDL_Texture*			ghost_tex;
	
	UICursor*				cursor;	

	map<string, UIEntity*>	gui_database;

public:
	bool					debug;
	SDL_Rect				selection_rect;
	CURSOR_STATE			cursor_state;
	list<UIImage*>			mini_wireframes;
};

#endif