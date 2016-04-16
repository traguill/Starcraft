#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "j1Module.h"
#include "UICursor.h"

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

private:

	void LoadLevel();
	void SaveLevelDesign();

private:

	uint map_id; //Drawable map
	uint collider_id;

	bool game_paused;

	bool debug;

	uint music;

private:


};

#endif // __GAME_SCENE_H__