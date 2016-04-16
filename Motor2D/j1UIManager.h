#ifndef __J1UIMANAGER_H__
#define __J1UIMANAGER_H__

#include "j1Module.h"
#include "UIProgressBar.h"

class UILabel;
class UIImage;
class UIEntity;
class UIButton;
class UIInputBox;
class UICursor;
class UIProgressBar;
enum UNIT_TYPE;


enum CURSOR_STATE
{
	STANDARD,
	TO_RIGHT,
	TO_LEFT,
	UP,
	DOWN,
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


	bool LoadUiInfo();
	// Gui creation functions

	SDL_Texture* GetAtlas() const;

	//Creators ------------------------------------------------------------------------------------------------
	UILabel* CreateLabel(const char* text, const int x, const int y, j1Module* listener = NULL);

	UIImage* CreateImage(SDL_Rect _section, const int x, const int y, bool initial_visible, bool on_list = true,j1Module* listener = NULL);

	UIButton* CreateButton(const char* _text, const int x, const int y, SDL_Rect section_idle, SDL_Rect section_pressed, SDL_Rect section_hover, j1Module* listener = NULL);

	UICursor* CreateCursor(vector<SDL_Rect> sections, float anim_speed, j1Module* listener = NULL);

	UIProgressBar* CreateBar(string _type, int max_num, const int x, const int y, j1Module* listener = NULL);

	void CreateMiniWireframe(UNIT_TYPE type, uint pos);

	void DeleteMiniWIreframe(uint pos);

	void OcultWireframes();

	//UIInputBox* CreateInputBox(const char* text, const int x, const int y, const char* path, j1Module* listener = NULL);
	//Functions ---------------------------------------------------------------------------------------------------
	UIEntity* GetMouseHover()const;

	void EraseElement(UIEntity* entity);

private:
	//Utilities ------------------------------------------------------------------------------------------------------
	void GetMouseInput(); //Get input of mouse to drag windows/elements
	void SetNextFocus(); //Sets the focus to the next element
	void ShowIndividualWireframe();
	void ShowMiniWireframes(float dt);
	void RectangleSelection();


private:

	bool					pressed_last_frame = false;
	SDL_Texture*			atlas;
	string					atlas_file_name;

	list<UIEntity*>			gui_elements;
	UIEntity*				gui_pressed = NULL;
	UIEntity*				focus = NULL;

	list<UIProgressBar*>	ui_progress_bar;

	//ui unit sprites
	SDL_Texture*			ui_icons;
	SDL_Texture*			move_ui;

	//It maybe has to be public
	SDL_Texture*			rects;

	UICursor*				cursor;
	UILabel*				life_HUD;
	string					ui_file_path;

	//Marine
	UIImage*				marine_weapon_icon;
	UIImage*				marine_armour_icon;
	UIImage*				marine_wireframe;

	//Ghost
	UIImage*				ghost_weapon_icon;
	UIImage*				ghost_wireframe;

	//Firebat
	UIImage*				firebat_weapon_icon;
	UIImage*				firebat_wireframe;

	//Medic
	UIImage*				 medic_wireframe;

	//Observer
	UIImage*				observer_wireframe;
	

public:
	bool					debug;
	SDL_Rect				selection_rect;
	CURSOR_STATE			cursor_state;
	list<UIImage*>			mini_wireframes;
};

#endif