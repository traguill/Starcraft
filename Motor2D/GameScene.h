#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "j1Module.h"
#include "UICursor.h"
#include "UILabel.h"
#include "UIButton.h"
#include "j1Timer.h"

struct SDL_Texture;

class GameScene : public j1Module
{
public:

	GameScene();

	// Destructor
	virtual ~GameScene();

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

	bool GamePaused()const;

	void OnGUI(UIEntity* gui, GUI_EVENTS event);

private:

	void LoadLevel();
	void SaveLevelDesign();


private:

	uint map_id; //Drawable map
	uint collider_id;

	bool game_paused;

	bool debug;

	uint music;

public:

	//It maybe has to be public

	UILabel*				life_HUD;

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

	//Sniper
	UIImage*                snipper_ui;

	//EVENT MANAGER
	UILabel* objective_info_1;
	UILabel* objective_info_2;
	UILabel* objective_info_3;
	UIImage* objectives_box;
	UIImage* pause_mark;
	UIImage* run_mark;

	//Pathfinding Label
	UILabel* pathfinding_label;
	j1Timer parthfinding_label_timer;

	//Bomb
	SDL_Texture* bomb;
	iPoint bomb_pos;
	SDL_Rect bomb_rect;
	bool bomb_available;

	SDL_Rect bomb_zone;

	//Win UI
	UIImage* win_background;
	UIButton* win_button;

	//Loose UI
	UIImage* loose_background;
	UIButton* loose_button;

};

#endif // __GAME_SCENE_H__