#ifndef __CREDIT_SCENE_H__
#define __CREDIT_SCENE_H__

#include "j1Module.h"
#include "Animation.h"


class UIImage;
class UILabel;

class CreditScene : public j1Module
{
public:

	CreditScene();

	// Destructor
	virtual ~CreditScene();

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

	//void OnGUI(UIEntity* gui, GUI_EVENTS event);

private:

	UIImage* sheep_logo;
	UIImage* upc_logo;
	UIImage* credits;
	UIImage* project;
	UIImage* license_blizzard;
	UIImage* license_image;

	list<UIImage*>	credits_list;

	j1Timer			credits_timer;
	bool			start_credit = false;

	bool close_game;


};

#endif // __CREDIT_SCENE_H__