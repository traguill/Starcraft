#ifndef __UNIT_H__
#define __UNIT_H__

#include "TacticalAI.h"
#include "p2Defs.h"
#include "p2Point.h"
#include "Entity.h"
#include "UIProgressBar.h"
#include <vector>
#include <queue>

#define MOVE_RADIUS 8

#define INVISIBILITY_ALPHA 95 

class Bullet;

struct ConePoint
{
	iPoint point;
	iPoint point_a;
	iPoint point_b;

	ConePoint();
	ConePoint(iPoint p);
};

enum UNIT_TYPE
{ 
	MARINE, 
	FIREBAT, 
	GHOST, 
	MEDIC, 
	OBSERVER
};

enum UNIT_STATE
{
	UNIT_IDLE, 
	UNIT_MOVE,
	UNIT_ATTACK,
	UNIT_DIE
};

enum UNIT_ABILITY
{
	INVISIBLE,
	SNIPPER,
	HEAL
};

class Unit : public Entity
{
	friend class j1EntityManager;
	friend class Ghost;
	friend class Marine;

public:

	Unit();
	Unit(Unit* u, bool is_enemy);

	~Unit();


	virtual void Update(float dt);
	virtual void Draw();

	void SetPath(vector<iPoint> _path);
	void AddPath(vector<iPoint> _path); //Adds the path to the existing one combining them
	vector<iPoint> GetPath()const;
	void SetPathId(uint id);

	fPoint GetDirection()const; //Returns the direction in form of a vector. Ex: (1,1) -north-east (-1) south etc

	UNIT_TYPE GetType()const;
	uint GetRange()const;

	void SetTarget(Unit* unit);
	Unit* GetTarget();

	void DiscardTarget(); //Stops attacking the target for some reason

	bool IsVisible()const;

	void ApplyDamage(uint dmg, Unit* source, Bullet* bullet = NULL);

	int GetLife()const;
	int GetMana()const;

	int GetMaxLife()const;
	int GetMaxMana()const;

	SDL_Texture* GetAuxiliarTexture() const;

	virtual void CastAbility(const UNIT_ABILITY ability); //Actual method to USE the ability

protected:
	virtual void SetAnimation();

private:

	void Move(float dt);
	virtual void Attack(float dt);
	void SetDirection();
	void CenterUnit();

	bool CheckTargetRange();

	void Delete(); 

	void UseAbility(uint id); //Unit use the ability asigned to the id number
	

	//AsignPath with offset
	void AsignPath(vector<iPoint> main_path);

	void AlertNearUnits(iPoint destination, list<Unit*> list); //Near units will go to destination point

	//Draw Vision Cone
	void DrawVisionCone();
	vector<iPoint> CollidersInsideConeVision(fPoint p0, fPoint p2, fPoint p3);
	void GetKeyPointsConeVision(vector<iPoint>& points, vector<ConePoint>& key_points,const fPoint& origin);
	bool HitAdjacentTile(iPoint origin, iPoint hit);
	bool CheckAdjacent(const iPoint& point);
	void ConnectKeyPoints(vector<ConePoint>& list);
	bool FindInVector(const iPoint& point, const vector<iPoint>& list);
	bool ConnectDirections(const iPoint& point, const vector<ConePoint>& list, bool by_x = false, bool by_y = false); //Search near connections by same x or y or both

protected:

	SDL_Texture* auxiliar_texture;
	uint speed;
	uint damage;
	uint friendly_damage;
	uint friendly_life;
	uint range;
	uint cool;
	vector<iPoint> path;
	bool costume;
	bool selected = false;
	bool invisible = false;
	Unit* target = NULL;

	float max_mana;
	float mana;
	int mana_regen;

	list<Unit*> attacking_units; //Units that are attacking me
	list<UNIT_ABILITY> abilities;

public:
	UNIT_STATE state;
	queue<UNIT_EVENT> events;
	uint vision;

	//Can't change state when is resolving a collision or do another thing
	bool avoid_change_state = false;
	UNIT_TYPE type;
	bool is_enemy;

	//fx
	uint death_fx;
	uint attack_fx;

	//Animations
	//MOVE
	Animation up;
	Animation down;
	Animation right;
	Animation left;
	Animation up_right;
	Animation down_right;
	Animation up_left;
	Animation down_left;
	//IDLE
	Animation i_up;
	Animation i_down;
	Animation i_right;
	Animation i_left;
	Animation i_up_right;
	Animation i_down_right;
	Animation i_up_left;
	Animation i_down_left;
	//ATTACK
	Animation a_up;
	Animation a_down;
	Animation a_right;
	Animation a_left;
	Animation a_up_right;
	Animation a_down_right;
	Animation a_up_left;
	Animation a_down_left;
	//DEATH
	iPoint death_pos_corrector;
	iPoint death_size;
	Animation death;

	Animation* current_animation;
	float walk_anim_speed;
	float idle_anim_speed;
	float attack_anim_speed;
	float death_anim_speed;

	//Pathfinding
	fPoint direction;
	bool has_destination = false;
	iPoint	dst_point; //Tile point
	//Path
	uint path_id;
	int path_offset_x; //Offset to the original path
	int path_offset_y;
	bool waiting_for_path = false;
	//Patrol
	bool patrol;
	iPoint original_point;
	fPoint original_direction;
	vector<iPoint> patrol_path;

	//Attacking
	float cool_timer = 0;
};
#endif