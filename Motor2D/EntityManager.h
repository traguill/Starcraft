#ifndef __ENTITYMANAGER_H__
#define __ENTITYMANAGER_H__

#include "j1Module.h"
#include "Entity.h"
#include <map>

using namespace std;

// ---------------------------------------------------
class j1EntityManager : public j1Module
{
public:

	j1EntityManager();

	// Destructor
	virtual ~j1EntityManager();

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

private:


public:
	//the key could be a int
	//Entity* must be Unit*
	map<string, Entity*> friendly_units; 
	list<Entity*> hostile_enities;

};

#endif