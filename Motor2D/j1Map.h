#ifndef __j1MAP_H__
#define __j1MAP_H__

#include <list>
#include <map>
#include "PugiXml/src/pugixml.hpp"
#include "j1Module.h"

// ----------------------------------------------------
struct Properties
{
	struct Property
	{
		string name;
		int value;
	};

	~Properties()
	{
		list<Property*>::iterator i = list_p.begin();

		while (i != list_p.end())
		{
			delete (*i);
			++i;
		}

		list_p.clear();
	}

	int Get(const char* name, int default_value = 0) const;

	list<Property*>	list_p;
};

// ----------------------------------------------------
struct MapLayer
{
	string	name;
	int			width;
	int			height;
	uint*		data;
	Properties	properties;

	MapLayer() : data(NULL)
	{}

	~MapLayer()
	{
		delete[] data;
		data = NULL;
	}

	inline uint Get(int x, int y) const
	{
		return data[(y*width) + x];
	}
};

// ----------------------------------------------------
struct TileSet
{
	SDL_Rect GetTileRect(int id) const;

	string				name;
	int					firstgid;
	int					margin;
	int					spacing;
	int					tile_width;
	int					tile_height;
	SDL_Texture*		texture;
	int					tex_width;
	int					tex_height;
	int					num_tiles_width;
	int					num_tiles_height;
	int					offset_x;
	int					offset_y;
};

enum MapTypes
{
	MAPTYPE_UNKNOWN = 0,
	MAPTYPE_ORTHOGONAL,
	MAPTYPE_ISOMETRIC,
	MAPTYPE_STAGGERED
};
// ----------------------------------------------------
struct MapData
{
	int					width;
	int					height;
	int					tile_width;
	int					tile_height;
	SDL_Color			background_color;
	MapTypes			type;
	list<TileSet*>		tilesets;
	list<MapLayer*>		layers;
};

// ----------------------------------------------------
class j1Map : public j1Module
{
public:

	j1Map();

	// Destructor
	virtual ~j1Map();

	// Called before render is available
	bool Awake(pugi::xml_node& conf);

	// Called each loop iteration
	void Draw(int id);

	// Called before quitting
	bool CleanUp();

	// Load new map and get the id of the map
	bool Load(const char* path,uint &id);
	void UnLoad(const uint& id);

	iPoint MapToWorld(int x, int y, uint map_id);
	iPoint WorldToMap(int x, int y, uint map_id);
	bool CreateWalkabilityMap(int& width, int& height, uchar** buffer,uint map_id) const;

private:

	bool LoadMap(MapData* data);
	bool LoadTilesetDetails(pugi::xml_node& tileset_node, TileSet* set);
	bool LoadTilesetImage(pugi::xml_node& tileset_node, TileSet* set);
	bool LoadLayer(pugi::xml_node& node, MapLayer* layer);
	bool LoadProperties(pugi::xml_node& node, Properties& properties);

	TileSet* GetTilesetFromTileId(int id, uint map_id)const;

public:

	map<uint, MapData*> maps;

private:

	pugi::xml_document	map_file;
	string				folder;
	bool				map_loaded;
};

#endif // __j1MAP_H__