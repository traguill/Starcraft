#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1PathFinding.h"


j1PathFinding::j1PathFinding() :
j1Module(),
map(NULL),
width(0),
height(0)
{
	name.append("pathfinding");
}

// Destructor
j1PathFinding::~j1PathFinding()
{
	delete[] map;
	map = NULL;
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

bool j1PathFinding::PreUpdate()
{
	//timer.Start();

	int paths_calculated = 0;
	int paths_deleted = 0;
	int paths_to_process = paths_to_calculate.size();

	int iterations = 0;
	bool can_calculate = true;
	std::map<uint, Path*>::iterator path = paths_to_calculate.begin();

	while (path != paths_to_calculate.end())
	{
		if (path->second->completed == false)
		{
			if (can_calculate)
			{
				iterations = CalculatePath(path->second, 8 - iterations);

				++paths_calculated;

				if (iterations >= 8)
					can_calculate = false;
			}
 			
		}
		else
		{
			delete path->second;
			path->second = NULL;
			std::map<uint, Path*>::iterator tmp = path;
			++path;
			paths_to_calculate.erase(tmp);

			++paths_deleted;
		}
		++path;
	}

	/*if (paths_to_process > 0)
		LOG("Pathfinding time %d, Iterations %i,  Paths to process: %i, Paths calculated %i, Paths deleted %i", 
			timer.Read(), iterations, paths_to_process, paths_calculated, paths_deleted);*/


	return true;
}

bool j1PathFinding::PostUpdate()
{

	return true;
}

// Called before quitting
bool j1PathFinding::CleanUp()
{
	LOG("Freeing pathfinding library");

	std::map<uint, Path*>::iterator path = paths_to_calculate.begin();

	while (path != paths_to_calculate.end())
	{
		delete path->second;
		path->second = NULL;
		std::map<uint, Path*>::iterator tmp = path;
		++path;
		paths_to_calculate.erase(tmp);
	}

	paths_to_calculate.clear();

	delete[] map;
	map = NULL;
	return true;
}

// Set the map to do the pathfinding
void j1PathFinding::SetMap(uint width, uint height, uchar* data)
{
	this->width = width;
	this->height = height;

	delete[] map;
	map = NULL;
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
	int min = 6500535;
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

void PathNode::IdentifySuccessors(PathList& successors, iPoint startNode, iPoint endNode, j1PathFinding* path_finder)const
{
	PathList neighbours;
	this->FindWalkableAdjacents(neighbours, path_finder);

	list<PathNode>::iterator neighbour = neighbours.list_nodes.begin();

	while (neighbour != neighbours.list_nodes.end())
	{
		int dx = clamp(neighbour->pos.x - this->pos.x, -1, 1);
		int dy = clamp(neighbour->pos.y - this->pos.y, -1, 1);

		PathNode jump_point(-1, -1, iPoint(-1, -1), this);
		bool succed = path_finder->Jump(this->pos.x, this->pos.y, dx, dy, startNode, endNode, jump_point);

		if (succed == true)
			successors.list_nodes.push_back(jump_point);

		++neighbour;
	}
}

bool j1PathFinding::Jump(int cx, int cy, int dx, int dy, iPoint start, iPoint end,PathNode& new_node)
{
	iPoint next(cx + dx, cy + dy);

	if (IsWalkable(next) == false)
		return false;

	if (next.x == end.x && next.y == end.y)
	{
		new_node.pos = next;
		return true;
	}

	//Diagonal
	if (dx != 0 && dy != 0)
	{
		if (!IsWalkable(iPoint(cx + dx, cy)))
		{
			new_node.pos = next;
			return true;
		}
		if (!IsWalkable(iPoint(cx, cy + dy)))
		{
			new_node.pos = next;
			return true;
		}

		//Check also horizontal and vertical directions for forced neighbors
		if (Jump(next.x, next.y, dx, 0, start, end, new_node) != NULL || Jump(next.x, next.y, 0, dy, start, end, new_node) != NULL)
		{
			new_node.pos = next;
			return true;
		}
	}
	else
	{
		//Horizontal
		if (dx != 0)
		{
			if (!IsWalkable(iPoint(cx, cy + 1)))
			{
				if (IsWalkable(iPoint(cx + dx, cy + 1)))
				{
					new_node.pos = next;
					return true;
				}
			}
			else if (!IsWalkable(iPoint(cx, cy - 1)))
			{
				if (IsWalkable(iPoint(cx + dx, cy - 1)))
				{
					new_node.pos = next;
					return true;
				}
			}
		}
		else //Vertical
		{
			if (!IsWalkable(iPoint(cx + 1, cy)))
			{
				if (IsWalkable(iPoint(cx + 1, cy + dy)))
				{
					new_node.pos = next;
					return true;
				}
			}
			else if (!IsWalkable(iPoint(cx - 1, cy)))
			{
				if (IsWalkable(iPoint(cx - 1, cy + dy)))
				{
					new_node.pos = next;
					return true;
				}
			}
		}

	}

	//Forced neighbour not found try next jump point
	return Jump(next.x, next.y, dx, dy, start, end, new_node);
}

uint PathNode::FindWalkableAdjacents(PathList& list_to_fill, j1PathFinding* path_finder) const
{
	iPoint cell;
	uint before = list_to_fill.list_nodes.size();

	// north
	cell.create(pos.x, pos.y - 1);
	if (path_finder->IsWalkable(cell))
		list_to_fill.list_nodes.push_back(PathNode(-1, -1, cell, this));

	//north-east
	cell.create(pos.x + 1, pos.y - 1);
	if (path_finder->IsWalkable(cell))
		list_to_fill.list_nodes.push_back(PathNode(-1, -1, cell, this));

	// east
	cell.create(pos.x + 1, pos.y);
	if (path_finder->IsWalkable(cell))
		list_to_fill.list_nodes.push_back(PathNode(-1, -1, cell, this));

	//south-east
	cell.create(pos.x + 1, pos.y + 1);
	if (path_finder->IsWalkable(cell))
		list_to_fill.list_nodes.push_back(PathNode(-1, -1, cell, this));

	// south
	cell.create(pos.x, pos.y + 1);
	if (path_finder->IsWalkable(cell))
		list_to_fill.list_nodes.push_back(PathNode(-1, -1, cell, this));

	//south-west
	cell.create(pos.x - 1, pos.y + 1);
	if (path_finder->IsWalkable(cell))
		list_to_fill.list_nodes.push_back(PathNode(-1, -1, cell, this));
	
	// west
	cell.create(pos.x - 1, pos.y);
	if (path_finder->IsWalkable(cell))
		list_to_fill.list_nodes.push_back(PathNode(-1, -1, cell, this));

	//nord-west
	cell.create(pos.x - 1, pos.y - 1);
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
	g = parent->g + this->pos.DistanceManhattan(parent->pos);
	h = pos.DistanceManhattan(destination);

	return g + h;
}

iPoint j1PathFinding::FindNearestWalkable(const iPoint& origin)
{
	iPoint ret(origin);
	//Find the nearest walkable tile in a 10 tiles radius
	int var = 1;
	while (var != 10)
	{
		for (int dx = -var; dx < var; dx++)
		{
			for (int dy = -var; dy < var; dy++)
			{
				ret.x = origin.x + dx;
				ret.y = origin.y + dy;
				if (IsWalkable(ret))
					return ret;
			}
		}

		++var;
	}
	

	return ret.create(-1, -1);
}

// Actual A* algorithm -----------------------------------------------
int j1PathFinding::CreatePath(const iPoint& origin, const iPoint& destination)
{
	int ret = -1;

	iPoint actual_origin = origin;

	if (IsWalkable(destination))
	{
		if (IsWalkable(origin) == false)
		{
			actual_origin = FindNearestWalkable(origin);

			if (IsWalkable(actual_origin))
			{
				LOG("Found new origin walkable");
			}

			if (actual_origin.x == -1 && actual_origin.y == -1)
			{
				LOG("Path no created: Origin no walkable && no near walkable tiles available");
				return ret;
			}
				
		}
		Path* path = new Path();

		paths_to_calculate.insert(pair<uint, Path*>(++current_id, path));

		// Start pushing the origin in the open list
		path->open.list_nodes.push_back(PathNode(0, 0, actual_origin, NULL));

		path->origin = actual_origin;
		path->destination = destination;

		//CalculatePath(path);

		ret = current_id; //Id of the path created
	}

	return ret;
}

int j1PathFinding::CalculatePath(Path* path, int max_iterations)
{
	int it_time = 0;

	do
	{
		//Debug
		timer.Start();

		// Move the lowest score cell from open list to the closed list
		list<PathNode>::iterator lowest = path->open.GetNodeLowestScore();
		path->closed.list_nodes.push_back(*lowest);
		path->open.list_nodes.erase(lowest);
		list<PathNode>::iterator node = --path->closed.list_nodes.end();


		// If destination was added, we are done!
		if (node->pos == path->destination)
		{
			path->path_finished.clear();
			// Backtrack to create the final path
			const PathNode* path_node = &(*node);

			while (path_node)
			{
				path->path_finished.push_back(path_node->pos);
				path_node = path_node->parent;
			}

			iPoint* start = &path->path_finished[0];
			iPoint* end = &path->path_finished[path->path_finished.size() - 1];

			while (start < end)
				SWAP(*start++, *end--);

			path->completed = true;

			break;
		}

		// Fill a list with all adjacent nodes
		path->adjacent.list_nodes.clear();
		node->IdentifySuccessors(path->adjacent, path->origin, path->destination, this);


		list<PathNode>::iterator i = path->adjacent.list_nodes.begin();

		while (i != path->adjacent.list_nodes.end())
		{
			if (path->closed.Find(i->pos) != path->closed.list_nodes.end())
			{
				++i;
				continue;
			}

			list<PathNode>::iterator adjacent_in_open = path->open.Find(i->pos);

			if (adjacent_in_open == path->open.list_nodes.end())
			{
				i->CalculateF(path->destination);
				path->open.list_nodes.push_back(*i);
			}
			else
			{
				if (adjacent_in_open->g > i->g + 1)
				{
					adjacent_in_open->parent = i->parent;
					adjacent_in_open->CalculateF(path->destination);
				}
			}
			++i;
		}

		it_time += timer.Read();

		if (it_time >= max_iterations)
			break;

	} while (path->open.list_nodes.size() > 0);

	return it_time;
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
			{
				hitted_tile.x = x;
				hitted_tile.y = y;
				return false;
			}
				
			
		}
		else
		{
			if (IsWalkable(iPoint(x, y)) == false)
			{
				hitted_tile.x = x;
				hitted_tile.y = y;
				return false;
			}
			
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



iPoint j1PathFinding::GetLineTile()const
{
	return hitted_tile;
}

bool j1PathFinding::PathFinished(uint id)const
{
	std::map<uint, Path*>::const_iterator result = paths_to_calculate.find(id);

	if (result == paths_to_calculate.end())
	{
		LOG("PathFinding ERROR: wrong id to check path status");
		return false;
	}

	return result->second->completed;
}

vector<iPoint> j1PathFinding::GetPath(uint id)const
{
	vector<iPoint> ret;
	std::map<uint, Path*>::const_iterator result = paths_to_calculate.find(id);

	if (result == paths_to_calculate.end())
	{
		LOG("PathFinding ERROR: wrong id to get Path");
	}
	else
	{
		ret = result->second->path_finished;
	}

	return ret;
}

Path::Path()
{
	completed = false;
}