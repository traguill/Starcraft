#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "j1Module.h"
#include "UICursor.h"
#include "UILabel.h"
#include "UIButton.h"
#include "UIMiniMap.h"
#include "j1Timer.h"
#include <queue>

struct SDL_Texture;
struct SDL_Rect;
enum UNIT_TYPE;

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

	void SelectFX(UNIT_TYPE type);
	void MoveFX(UNIT_TYPE type);
	void AttackFX(UNIT_TYPE type);

	bool GetTutorialState();
	bool GetFinishedGame();

	void LoseGameDetected();
	void LoseGameKilled();


	bool IsGhostSelected();

private:

	void LoadGame(const char* path);
	void SaveGame(const char* path);

	void LoadAudio();

	void LoadHUD();

	void LoadQuitUI();

	void LoadTutorial();

	bool ExtractionPoint();

	void DisableMenu();

private:

	uint map_id; //Drawable map
	uint collider_id;

	bool game_paused;
	bool menu_ingame;

	bool debug;

	uint music;

	bool game_finished;
	bool tutorial_finished;

public:

	//Minimap
	UIMiniMap* minimap;

	//It maybe has to be public
	UIImage*				tutorial_window;
	UILabel*				tutorial_text;
	UILabel*				secondary_text;
	UIButton*				tutorial_button;
	UIButton*				skip_button;
	UIImage*				tutorial_image;
	UIImage*				tutorial_fadeblack;
	queue<SDL_Rect>			tutorial_images_queue;
	queue<char*>			tutorial_text_queue;
	queue<char*>			secondary_text_queue;

	//Quit Game
	UIImage*				quit_fadeblack;
	//UIImage*				quit_window;
	UIButton*				quit_button;
	UIButton*				resume_button;

	//UILabel*				life_HUD;
	j1Timer					pause_timer;
	bool					active_timer = false;

	//Marine
	UIImage*				marine_weapon_icon;
	UIImage*				marine_armour_icon;
	UIImage*				marine_wireframe;

	//Ghost
	UIImage*				ghost_weapon_icon;
	UIImage*				ghost_wireframe;
	//Buttons
	UIButton*				ghost_invisibility_button;
	UIButton*				ghost_snipermode_button;

	//Firebat
	UIImage*				firebat_weapon_icon;
	UIImage*				firebat_wireframe;

	//Medic
	UIImage*				 medic_wireframe;

	//Observer
	UIImage*				observer_wireframe;

	//Sniper
	j1Timer					sniper_ui_timer;
	UIImage*                snipper_ui;

	//EVENT MANAGER
	UILabel* objective_info_1;
	UILabel* objective_info_2;
	UILabel* objective_info_3;
	UIImage* objectives_box;
	UIImage* pause_mark;
	UIImage* run_mark;
	UILabel* sniper_ammo_label;
	UILabel* grenade_ammo_label;

	//Pathfinding Label
	UILabel* pathfinding_label;
	j1Timer parthfinding_label_timer;

	UILabel* game_saved;
	j1Timer game_saved_timer;

	//Bomb
	SDL_Texture* bomb;
	SDL_Texture* bzone;

	list<iPoint> bomb_pos;
	SDL_Rect bomb_rect;

	SDL_Rect bomb_zone;

	//Win UI
	UIImage* win_background;
	UIButton* win_button;

	//Loose UI
	UIImage* loose_background_detected;
	UIImage* loose_background_killed;
	UIImage* loose_background_hard;
	UIButton* loose_button;

	// SOUNDS FX

	//Select
	uint marine_select;
	uint ghost_select;
	uint firebat_select;
	uint medic_select;
	uint observer_select;

	//order
	uint marine_order;
	uint ghost_order;
	uint firebat_order;
	uint medic_order;
	uint observer_order;

	//attack order
	uint marine_attack_order;
	uint ghost_attack_order;
	uint firebat_attack_order;
	uint medic_attack_order;
	uint observer_attack_order;

	//death
	uint marine_death;
	uint ghost_death;
	uint firebat_death;
	uint medic_death;
	uint observer_death;

	//ammo
	uint sniper_ammo;
	uint intel_left;
	UILabel* no_ammo;

	//Energy
	UILabel* no_energy;
};

#endif // __GAME_SCENE_H__