#ifndef __ENTITYMANAGER_H__
#define __ENTITYMANAGER_H__

#include "j1Module.h"
#include "Entity.h"
#include "Unit.h"
#include "Bullet.h"
#include <map>

#define COLLIDER_MAP 2

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

	void CreateUnit(UNIT_TYPE type, int x, int y, bool is_enemy, bool patrolling, vector<iPoint> point_path);

	void RemoveUnit(Unit* _unit);
	void RemoveBullet(Bullet* _bullet);

	void CleanUpList();

	string UnitTypeToString(UNIT_TYPE type)const;
	UNIT_TYPE UnitTypeToEnum(string type)const;

private:

	//Load data
	bool LoadUnitsInfo();
	void LoadBulletInfo(pugi::xml_node& units);
	void LoadUnitInfo(pugi::xml_node& units);
	void LoadUnitAnimationInfo(pugi::xml_node& unit, Unit* unit_db);
	void LoadSounds();

	//Selection
	void SelectUnits();
	void CheckUnderCursor();

	//Abilities
	void ActivateAbilities();

	//Pathfinding
	void SetMovement();
	void CalculateMovementRect();
	void AssignPath(Unit* u, uint path_id, iPoint* center); //For pathfinding id ->Need to wait
	void AssignPath(Unit* unit, vector<iPoint> path, iPoint* center); //For lines


	//Removing
	void DestroyUnit(Unit* _unit);
	void DestroyBullet(Bullet* _bullet);

	//DEBUG
	void PrintUnitDatabase()const;
	void Debug();

private:

	//Unit base to clone to create new units
	string units_file_path;
	map<string, Unit*>	units_database;

	//Select units
	iPoint select_start;
	iPoint select_end;
	SDL_Rect selection_rect;

	//Movement rectangle of selected units (world)
	SDL_Rect move_rec;
	iPoint center;

	//Remove
	list<Unit*> units_to_remove;
	list<Bullet*> bullets_to_remove;

public:
	//Need another list for buildings
	list<Unit*> friendly_units;
	list<Unit*> enemy_units;
	list<Unit*> selected_units;
	bool debug;

	SDL_Texture* gui_cursor;

	bool SNIPPER_MODE = false;

	//Bullet time
	float bullet_time = 1.0f;

	//Sniping
	list<Bullet*> bullets;

	//Costs
	float invisibility_cost;
	int snipper_cost;

	//Reference bullet
	Bullet* db_bullet = NULL;

	//Audio
	uint sound_shoot;
	uint sound_sniper_mode;
};

#endif