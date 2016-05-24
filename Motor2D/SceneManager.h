#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include "j1Module.h"

enum SCENES
{
	MENU,
	GAME,
	DEV
};


class SceneManager : public j1Module
{
public:

	SceneManager();

	// Destructor
	virtual ~SceneManager();

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


public:

	void WantToChangeScene(SCENES scene);

private:

	void DisableScene(SCENES scene);
	void EnableScene(SCENES scene);

	//Enable/Disable scenes ---------------------------------
	void EnableMenu();
	void DisableMenu();

	void EnableGame();
	void DisableGame();

	void EnableDev();
	void DisableDev();

public:

	bool changing_scene = false;
	bool dificulty = false;
	bool pro = false;
	bool in_game = false;

private:

	SCENES actual_scene; //Scene that we are now
	SCENES new_scene; //Scene that we want to load


};

#endif // __SCENE_MANAGER_H__