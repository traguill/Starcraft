#ifndef __DEV_SCENE_H__
#define __DEV_SCENE_H__

#include "j1Module.h"
#include "UICursor.h"
#include "UILabel.h"
#include "UIButton.h"
#include "j1Timer.h"

struct SDL_Texture;
enum UNIT_TYPE;
class Unit;

#define CAMERA_SPEED 1000

class DevScene : public j1Module
{
public:

	DevScene();

	// Destructor
	virtual ~DevScene();

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

	void UnitCreation();
	void UnitMovement();
	void BombMovement();

	void DrawSelection();
	void DeleteUnits();

	void AsignPatrol();
	void SetDirection();

	//Utilities
	bool PointInRect(iPoint p, SDL_Rect rec);
	bool Find(Unit* u);

private:

	uint map_id; //Drawable map
	uint collider_id;

	bool game_paused;

	bool debug;

	list<Unit*> selected_units;

	//Bomb
	SDL_Texture* bomb;
	iPoint bomb_pos;
	SDL_Rect bomb_rect;

	SDL_Rect bomb_zone;


public:
	

};

#endif 