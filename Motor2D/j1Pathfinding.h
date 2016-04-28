#ifndef __j1PATHFINDING_H__
#define __j1PATHFINDING_H__

#include "j1Module.h"
#include "p2Point.h"

#include <iostream>
#include <vector>
#include <map>

#define MAX_ITERATIONS 8
#define INVALID_WALK_CODE 255

struct PathNode;
struct PathList;
struct Path;
// --------------------------------------------------
class j1PathFinding : public j1Module
{
public:

	j1PathFinding();

	// Destructor
	virtual ~j1PathFinding();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before the first frame
	bool Start();

	bool PreUpdate();
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Set Map
	void SetMap(uint width, uint height, uchar* data);

	int CreatePath(const iPoint& origin, const iPoint& destination);

	bool CheckBoundaries(const iPoint& pos) const;
	bool IsWalkable(const iPoint& pos) const;
	uchar GetTileAt(const iPoint& pos) const;

	bool CreateLine(const iPoint& origin, const iPoint& destination);

	iPoint GetLineTile()const; //Returns the last hitted tile

	bool Jump(int cx, int cy, int dx, int dy, iPoint start, iPoint end, PathNode& new_node);

	//Check path status
	bool PathFinished(uint id)const;
	vector<iPoint> GetPath(uint id)const;

private:

	void CalculatePath(Path* path);
	
private:

	uint width;
	uint height;
	uchar* map;
	iPoint hitted_tile;
	std::map<uint, Path*> paths_to_calculate;

	uint current_id = 0;

};



// Helper struct to reprsent a node in the path creation
struct PathNode
{
	PathNode();
	PathNode(int g, int h, const iPoint& pos, const PathNode* parent);
	PathNode(const PathNode& node);

	uint FindWalkableAdjacents(PathList& list_to_fill, j1PathFinding* path_finder) const;
	int Score() const;
	int CalculateF(const iPoint& destination);

	void IdentifySuccessors(PathList& list_to_fill, iPoint startNode, iPoint endNode, j1PathFinding* path_finder)const;

	int g;
	int h;
	iPoint pos;
	const PathNode* parent;
};

// Helper struct to include a list of path nodes
struct PathList
{
	bool Contains(const iPoint& point) const;
	list<PathNode>::iterator Find(const iPoint& point);
	list<PathNode>::iterator GetNodeLowestScore();

	list<PathNode> list_nodes;
};

struct Path
{
	Path();
	PathList open;
	PathList closed;
	PathList adjacent;

	iPoint origin;
	iPoint destination;

	vector<iPoint> path_finished;

	bool completed;
};

#endif // __j1PATHFINDING_H__