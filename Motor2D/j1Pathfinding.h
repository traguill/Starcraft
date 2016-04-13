#ifndef __j1PATHFINDING_H__
#define __j1PATHFINDING_H__

#include "j1Module.h"
#include "p2Point.h"

#include <vector>

#define DEFAULT_PATH_LENGTH 50
#define INVALID_WALK_CODE 255

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

	// Called before quitting
	bool CleanUp();

	// Set Map
	void SetMap(uint width, uint height, uchar* data);

	int CreatePath(const iPoint& origin, const iPoint& destination);

	const vector<iPoint>* GetLastPath() const;

	bool CheckBoundaries(const iPoint& pos) const;
	bool IsWalkable(const iPoint& pos) const;
	uchar GetTileAt(const iPoint& pos) const;

	bool CreateLine(const iPoint& origin, const iPoint& destination);

	bool CreateOptimizedPath(const iPoint& origin, const iPoint& destination, vector<iPoint>& path);

	iPoint GetLineTile()const; //Returns the last hitted tile

private:

	uint width;
	uint height;
	uchar* map;
	vector<iPoint> last_path;
	iPoint hitted_tile;

};

struct PathList;

// Helper struct to reprsent a node in the path creation
struct PathNode
{
	PathNode();
	PathNode(int g, int h, const iPoint& pos, const PathNode* parent);
	PathNode(const PathNode& node);

	uint FindWalkableAdjacents(PathList& list_to_fill, j1PathFinding* path_finder) const;
	int Score() const;
	int CalculateF(const iPoint& destination);

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



#endif // __j1PATHFINDING_H__