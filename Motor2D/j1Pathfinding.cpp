#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1PathFinding.h"


j1PathFinding::j1PathFinding() :
j1Module(),
map(NULL),
last_path(DEFAULT_PATH_LENGTH),
width(0),
height(0)
{
	name.append("pathfinding");
}

// Destructor
j1PathFinding::~j1PathFinding()
{
	RELEASE_ARRAY(map);
}

// Called before render is available
bool j1PathFinding::Awake(pugi::xml_node&)
{
	LOG("Init Pathfinding library");
	bool ret = true;

	return ret;
}

// Called before the first frame
bool j1PathFinding::Start()
{
	LOG("Start pathfinding");
	bool ret = true;

	return ret;
}

// Called before quitting
bool j1PathFinding::CleanUp()
{
	LOG("Freeing pathfinding library");

	last_path.clear();
	RELEASE_ARRAY(map);
	return true;
}

// Set the map to do the pathfinding
void j1PathFinding::SetMap(uint width, uint height, uchar* data)
{
	this->width = width;
	this->height = height;

	RELEASE_ARRAY(map);
	map = new uchar[width*height];
	memcpy(map, data, width*height);
}

bool j1PathFinding::CheckBoundaries(const iPoint& pos) const
{
	return (pos.x >= 0 && pos.x <= width &&
		pos.y >= 0 && pos.y <= height);
}

bool j1PathFinding::IsWalkable(const iPoint& pos) const
{
	uchar t = GetTileAt(pos);
	return t != INVALID_WALK_CODE && t > 0;
}

uchar j1PathFinding::GetTileAt(const iPoint& pos) const
{
	if (CheckBoundaries(pos))
		return map[(pos.y*width) + pos.x];

	return INVALID_WALK_CODE;
}

const vector<iPoint>* j1PathFinding::GetLastPath() const
{
	return &last_path;
}

// PathList ------------------------------------------------------------------------
list<PathNode>::iterator PathList::Find(const iPoint& point) 
{
	list<PathNode>::iterator i = list_nodes.begin();

	while (i != list_nodes.end())
	{
		if (i->pos == point)
		{
			return i;
		}
		++i;
	}

	return list_nodes.end();
}

list<PathNode>::iterator PathList::GetNodeLowestScore() 
{
	list<PathNode>::iterator ret = list_nodes.end();
	int min = 65535;
	list<PathNode>::iterator i = list_nodes.begin();

	while (i != list_nodes.end())
	{
		if (i->Score() < min)
		{
			min = i->Score();
			ret = i;
		}
		++i;
	}

	return ret;
}

// PathNode -------------------------------------------------------------------------
PathNode::PathNode() : g(-1), h(-1), pos(-1, -1), parent(NULL)
{}

PathNode::PathNode(int g, int h, const iPoint& pos, const PathNode* parent) : g(g), h(h), pos(pos), parent(parent)
{}

PathNode::PathNode(const PathNode& node) : g(node.g), h(node.h), pos(node.pos), parent(node.parent)
{}

uint PathNode::FindWalkableAdjacents(PathList& list_to_fill, j1PathFinding* path_finder) const
{
	iPoint cell;
	uint before = list_to_fill.list_nodes.size();

	// north
	cell.create(pos.x, pos.y + 1);
	if (path_finder->IsWalkable(cell))
		list_to_fill.list_nodes.push_back(PathNode(-1, -1, cell, this));

	//north-east
	cell.create(pos.x + 1, pos.y + 1);
	if (path_finder->IsWalkable(cell))
		list_to_fill.list_nodes.push_back(PathNode(-1, -1, cell, this));

	// east
	cell.create(pos.x + 1, pos.y);
	if (path_finder->IsWalkable(cell))
		list_to_fill.list_nodes.push_back(PathNode(-1, -1, cell, this));

	//south-east
	cell.create(pos.x + 1, pos.y - 1);
	if (path_finder->IsWalkable(cell))
		list_to_fill.list_nodes.push_back(PathNode(-1, -1, cell, this));

	// south
	cell.create(pos.x, pos.y - 1);
	if (path_finder->IsWalkable(cell))
		list_to_fill.list_nodes.push_back(PathNode(-1, -1, cell, this));

	//south-west
	cell.create(pos.x - 1, pos.y - 1);
	if (path_finder->IsWalkable(cell))
		list_to_fill.list_nodes.push_back(PathNode(-1, -1, cell, this));
	
	// west
	cell.create(pos.x - 1, pos.y);
	if (path_finder->IsWalkable(cell))
		list_to_fill.list_nodes.push_back(PathNode(-1, -1, cell, this));

	//nord-west
	cell.create(pos.x - 1, pos.y + 1);
	if (path_finder->IsWalkable(cell))
		list_to_fill.list_nodes.push_back(PathNode(-1, -1, cell, this));

	return list_to_fill.list_nodes.size();
}

int PathNode::Score() const
{
	return g + h;
}

int PathNode::CalculateF(const iPoint& destination)
{
	g = parent->g + 1;
	//h = pos.DistanceManhattan(destination);
	h = pos.DistanceNoSqrt(destination);

	return g + h;
}

// Actual A* algorithm -----------------------------------------------
int j1PathFinding::CreatePath(const iPoint& origin, const iPoint& destination)
{
	int ret = -1;
	int iterations = 0;

	if (IsWalkable(origin) && IsWalkable(destination))
	{
		PathList open;
		PathList closed;
		PathList adjacent;

		// Start pushing the origin in the open list
		open.list_nodes.push_back(PathNode(0, 0, origin, NULL));

		// Iterate while we have open destinations to visit
		do
		{
			// Move the lowest score cell from open list to the closed list
			list<PathNode>::iterator lowest = open.GetNodeLowestScore();
			closed.list_nodes.push_back(*lowest);
			iPoint pos = lowest->pos;
			open.list_nodes.erase(lowest);
			list<PathNode>::iterator node = closed.Find(pos);


			// If destination was added, we are done!
			if (node->pos == destination)
			{
				last_path.clear();
				// Backtrack to create the final path
				const PathNode* path_node = &(*node);

				while (path_node)
				{
					last_path.push_back(path_node->pos);
					path_node = path_node->parent;
				}

				iPoint* start = &last_path[0];
				iPoint* end = &last_path[last_path.size() - 1];

				while (start < end)
					SWAP(*start++, *end--);
				
				ret = last_path.size();
				LOG("Created path of %d steps in %d iterations", ret, iterations);
				break;
			}

			// Fill a list with all adjacent nodes
			adjacent.list_nodes.clear();
			node->FindWalkableAdjacents(adjacent, this);


			list<PathNode>::iterator i = adjacent.list_nodes.begin();

			while (i != adjacent.list_nodes.end())
			{
				if (closed.Find(i->pos) != closed.list_nodes.end())
				{
					++i;
					continue;
				}

				list<PathNode>::iterator adjacent_in_open = open.Find(i->pos);

				if (adjacent_in_open == open.list_nodes.end())
				{
					i->CalculateF(destination);
					open.list_nodes.push_back(*i);
				}
				else
				{
					if (adjacent_in_open->g > i->g + 1)
					{
						adjacent_in_open->parent = i->parent;
						adjacent_in_open->CalculateF(destination);
					}
				}
				++i;
			}

			
			

			++iterations;
		} while (open.list_nodes.size() > 0);
	}

	return ret;
}


bool j1PathFinding::CreateLine(const iPoint& origin, const iPoint& destination)
{
	bool ret = true;

	iPoint p1 = origin;
	iPoint p2 = destination;

	bool steep = (abs(p2.y - p1.y) > abs(p2.x - p1.x));

	if (steep)
	{
		swap(p1.x, p1.y);
		swap(p2.x, p2.y);
	}

	if (p1.x > p2.x)
	{
		swap(p1.x, p2.x);
		swap(p1.y, p2.y);
	}

	int dx = p2.x - p1.x;
	int dy = abs(p2.y - p1.y);

	int error = dx / 2;
	int ystep = (p1.y < p2.y) ? 1 : -1;
	int y = p1.y;

	const int max_x = p2.x;

	for (int x = p1.x; x < max_x; x++)
	{
		if (steep)
		{
			if (IsWalkable(iPoint(y, x)) == false)
				return false;
			
		}
		else
		{
			if (IsWalkable(iPoint(x, y)) == false)
				return false;
			
		}

		error -= dy;
		if (error < 0)
		{
			y += ystep;
			error += dx;
		}
	}




	return true;
}


