#ifndef __MENU_SCENE_H__
#define __MENU_SCENE_H__

#include "j1Module.h"
#include "Animation.h"


class UIImage;
class UIButton;

class MenuScene : public j1Module
{
public:

	MenuScene();

	// Destructor
	virtual ~MenuScene();

	// Called before render is available
	bool Awake(pugi::xml_node& conf);

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

	UIImage* background;
	Animation background_anim;
	UIImage* logo;

	bool close_game;

public:
	UIButton* start;
	UIButton* quit;
	UIButton* normal;
	UIButton* hardcore;
	UIButton* pro;



};

#endif // __GAME_SCENE_H__