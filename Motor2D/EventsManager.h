#ifndef __EVENTSMANAGER_H__
#define __EVENTSMANAGER_H__

#include "j1Module.h"
#include "j1UIManager.h"
#include "UILabel.h"
#include "j1Textures.h"
#include "UIImage.h"

enum CURRENT_EVENT
{
	INITIAL_STATE,
	BOMB_RETRIVED
};

// ---------------------------------------------------
class EventsManager : public j1Module
{
public:

	EventsManager();

	// Destructor
	virtual ~EventsManager();

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

	void BombRetrieved();

public:
	CURRENT_EVENT game_event = INITIAL_STATE;

};

#endif