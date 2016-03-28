#ifndef __ENTITYMANAGER_H__
#define __ENTITYMANAGER_H__

#include "j1Module.h"
#include "Entity.h"
#include "Unit.h"
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

public:

	Unit* CreateUnit(UNIT_TYPE type, int x, int y);
	
private:
	bool LoadUnitsInfo();

	string UnitTypeToString(UNIT_TYPE type)const;
	UNIT_TYPE UnitTypeToEnum(string type)const;

	//DEBUG
	void PrintUnitDatabase()const;

private:

	//Unit base to clone to create new units
	string units_file_path;
	map<string, Unit*>	units_database;


public:
	//the key could be a int
	//Entity* must be Unit*
	map<string, Unit*> friendly_units; 
	list<Entity*> hostile_enities;

};

#endif