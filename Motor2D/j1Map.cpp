#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1FileSystem.h"
#include "j1Textures.h"
#include "j1Map.h"
#include <math.h>

j1Map::j1Map() : j1Module(), map_loaded(false)
{
	name.append("map");
}

// Destructor
j1Map::~j1Map()
{}

// Called before render is available
bool j1Map::Awake(pugi::xml_node& config)
{
	LOG("Loading Map Parser");
	bool ret = true;

	folder.append(config.child("folder").child_value());

	return ret;
}

void j1Map::Draw(int id)
{
	map<uint, MapData*>::iterator map_it = maps.find(id);
	if (map_it == maps.end())
		return;

	list<MapLayer*>::iterator i = map_it->second->layers.begin();

	while (i != map_it->second->layers.end())
	{
		MapLayer* layer = *i;

		/*if (layer->properties.Get("Nodraw") != 0)
		{
			++i;
			continue;
		}*/

		SDL_Rect cam = App->render->camera;

		iPoint begin, end;

		begin = App->map->WorldToMap(-cam.x, -cam.y, id);
		end = App->map->WorldToMap((-cam.x) + cam.w, (-cam.y) + cam.h, id);
			
		for (int y = begin.y; y <= end.y; ++y)
		{
			for (int x = begin.x; x <= end.x; ++x)
			{
				int tile_id = layer->Get(x, y);
				if (tile_id > 0)
				{
					TileSet* tileset = GetTilesetFromTileId(tile_id, id);

					SDL_Rect r = tileset->GetTileRect(tile_id);
					iPoint pos = MapToWorld(x, y, id);

					SDL_Rect cam = App->render->camera;

					App->render->Blit(tileset->texture, pos.x, pos.y, &r);
				}
			}
		}
		++i;
	}

}

int Properties::Get(const char* value, int default_value) const
{
	list<Property*>::const_iterator i = list_p.begin();

	while (i != list_p.end())
	{
		if((*i)->name == value)
			return (*i)->value;
		++i;
	}

	return default_value;
}

TileSet* j1Map::GetTilesetFromTileId(int id, uint map_id) const
{
	map<uint, MapData*>::const_iterator map_it = maps.find(map_id);
	if (map_it == maps.end())
		return NULL;

	list<TileSet*>::const_iterator i = map_it->second->tilesets.begin();
	TileSet* set = (*i);

	while (i != map_it->second->tilesets.end())
	{
		if (id < (*i)->firstgid)
		{
			set = *--i; //get prev

			//Reset iterator
			++i;

			break;
		}
		set = (*i);
		++i;
	}

	return set;
}

iPoint j1Map::MapToWorld(int x, int y,uint map_id) 
{
	map<uint, MapData*>::iterator map_it = maps.find(map_id);

	iPoint ret;

	if (map_it == maps.end())
	{
		LOG("Wrong map id");
		ret.x = x; ret.y = y;
		return ret;
	}

	if(map_it->second->type == MAPTYPE_ORTHOGONAL)
	{
		ret.x = x * map_it->second->tile_width;
		ret.y = y * map_it->second->tile_height;
	}
	else if(map_it->second->type == MAPTYPE_ISOMETRIC)
	{
		ret.x = (x - y) * (map_it->second->tile_width * 0.5f);
		ret.y = (x + y) * (map_it->second->tile_height * 0.5f);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}

	return ret;
}

iPoint j1Map::WorldToMap(int x, int y, uint map_id)
{
	iPoint ret(0,0);

	map<uint, MapData*>::iterator map_it = maps.find(map_id);

	if (map_it == maps.end())
	{
		LOG("Wrong map id");
		return ret;
	}

	if(map_it->second->type == MAPTYPE_ORTHOGONAL)
	{
		ret.x = x / map_it->second->tile_width;
		ret.y = y / map_it->second->tile_height;
	}
	else if(map_it->second->type == MAPTYPE_ISOMETRIC)
	{
		
		float half_width = map_it->second->tile_width * 0.5f;
		float half_height = map_it->second->tile_height * 0.5f;
		ret.x = int( (x / half_width + y / half_height) / 2) - 1;
		ret.y = int( (y / half_height - (x / half_width)) / 2);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}

	return ret;
}

SDL_Rect TileSet::GetTileRect(int id) const
{
	int relative_id = id - firstgid;
	SDL_Rect rect;
	rect.w = tile_width;
	rect.h = tile_height;
	rect.x = margin + ((rect.w + spacing) * (relative_id % num_tiles_width));
	rect.y = margin + ((rect.h + spacing) * (relative_id / num_tiles_width));
	return rect;
}

// Called before quitting
bool j1Map::CleanUp()
{
	LOG("Unloading map");

	// Remove all tilesets
	map<uint, MapData*>::iterator map_it = maps.begin();

	while (map_it != maps.end())
	{
		list<TileSet*>::iterator i = map_it->second->tilesets.begin();

		while (i != map_it->second->tilesets.end())
		{
			delete *i;
			++i;
		}

		map_it->second->tilesets.clear();

		// Remove all layers

		list<MapLayer*>::iterator i2 = map_it->second->layers.begin();

		while (i2 != map_it->second->layers.end())
		{
			delete *i2;
			++i2;
		}

		map_it->second->layers.clear();

		++map_it;
	}


	// Clean up the pugui tree
	map_file.reset();

	return true;
}

// Load new map
bool j1Map::Load(const char* file_name, uint &id)
{
	bool ret = true;
	string tmp = folder.data();
	tmp = tmp + file_name;

	char* buf;
	int size = App->fs->Load(tmp.data(), &buf);
	pugi::xml_parse_result result = map_file.load_buffer(buf, size);

	RELEASE(buf);

	if(result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", file_name, result.description());
		ret = false;
	}

	MapData* map_data = new MapData();

	// Load general info ----------------------------------------------
	if(ret == true)
	{
		ret = LoadMap(map_data);
	}

	// Load all tilesets info ----------------------------------------------
	pugi::xml_node tileset;
	for(tileset = map_file.child("map").child("tileset"); tileset && ret; tileset = tileset.next_sibling("tileset"))
	{
		TileSet* set = new TileSet();

		if(ret == true)
		{
			ret = LoadTilesetDetails(tileset, set);
		}

		if(ret == true)
		{
			ret = LoadTilesetImage(tileset, set);
		}

		map_data->tilesets.push_back(set);
	}

	// Load layer info ----------------------------------------------
	pugi::xml_node layer;
	for(layer = map_file.child("map").child("layer"); layer && ret; layer = layer.next_sibling("layer"))
	{
		MapLayer* lay = new MapLayer();

		ret = LoadLayer(layer, lay);

		if(ret == true)
			map_data->layers.push_back(lay);
	}

	if(ret == true)
	{
		id = maps.size() + 1;
		maps.insert(pair<uint,MapData*>(id, map_data));

		LOG("Successfully parsed map XML file: %s", file_name);
		LOG("width: %d height: %d", map_data->width, map_data->height);
		LOG("tile_width: %d tile_height: %d", map_data->tile_width, map_data->tile_height);

		list<TileSet*>::iterator i = map_data->tilesets.begin();

		while (i != map_data->tilesets.end())
		{
			TileSet* s = *i;
			LOG("Tileset ----");
			LOG("name: %s firstgid: %d", s->name.data(), s->firstgid);
			LOG("tile width: %d tile height: %d", s->tile_width, s->tile_height);
			LOG("spacing: %d margin: %d", s->spacing, s->margin);
			++i;
		}

		list<MapLayer*>::iterator i2 = map_data->layers.begin();

		while (i2 != map_data->layers.end())
		{
			MapLayer* l = *i2;
			LOG("Layer ----");
			LOG("name: %s", l->name.data());
			LOG("tile width: %d tile height: %d", l->width, l->height);
			++i2;
		}

	}

	map_loaded = ret;

	return ret;
}

// Load map general properties
bool j1Map::LoadMap(MapData* data)
{
	bool ret = true;
	pugi::xml_node map = map_file.child("map");

	if(map == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'map' tag.");
		ret = false;
	}
	else
	{
		data->width = map.attribute("width").as_int();
		data->height = map.attribute("height").as_int();
		data->tile_width = map.attribute("tilewidth").as_int();
		data->tile_height = map.attribute("tileheight").as_int();
		string bg_color(map.attribute("backgroundcolor").as_string());

		data->background_color.r = 0;
		data->background_color.g = 0;
		data->background_color.b = 0;
		data->background_color.a = 0;

		if(bg_color.length() > 0)
		{
			string red = bg_color.substr(1, 1);
			string green = bg_color.substr(3, 1);
			string blue = bg_color.substr(5, 1);

			int v = 0;

			sscanf_s(red.data(), "%x", &v);
			if(v >= 0 && v <= 255) data->background_color.r = v;

			sscanf_s(green.data(), "%x", &v);
			if(v >= 0 && v <= 255) data->background_color.g = v;

			sscanf_s(blue.data(), "%x", &v);
			if(v >= 0 && v <= 255) data->background_color.b = v;
		}

		string orientation(map.attribute("orientation").as_string());

		if(orientation == "orthogonal")
		{
			data->type = MAPTYPE_ORTHOGONAL;
		}
		else if(orientation == "isometric")
		{
			data->type = MAPTYPE_ISOMETRIC;
		}
		else if(orientation == "staggered")
		{
			data->type = MAPTYPE_STAGGERED;
		}
		else
		{
			data->type = MAPTYPE_UNKNOWN;
		}
	}

	return ret;
}

bool j1Map::LoadTilesetDetails(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	set->name.append(tileset_node.attribute("name").as_string());
	set->firstgid = tileset_node.attribute("firstgid").as_int();
	set->tile_width = tileset_node.attribute("tilewidth").as_int();
	set->tile_height = tileset_node.attribute("tileheight").as_int();
	set->margin = tileset_node.attribute("margin").as_int();
	set->spacing = tileset_node.attribute("spacing").as_int();
	pugi::xml_node offset = tileset_node.child("tileoffset");

	if(offset != NULL)
	{
		set->offset_x = offset.attribute("x").as_int();
		set->offset_y = offset.attribute("y").as_int();
	}
	else
	{
		set->offset_x = 0;
		set->offset_y = 0;
	}

	return ret;
}

bool j1Map::LoadTilesetImage(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	pugi::xml_node image = tileset_node.child("image");

	if(image == NULL)
	{
		LOG("Error parsing tileset xml file: Cannot find 'image' tag.");
		ret = false;
	}
	else
	{
		set->texture = App->tex->Load(PATH(folder.data(), image.attribute("source").as_string()));
		int w, h;
		SDL_QueryTexture(set->texture, NULL, NULL, &w, &h);
		set->tex_width = image.attribute("width").as_int();

		if(set->tex_width <= 0)
		{
			set->tex_width = w;
		}

		set->tex_height = image.attribute("height").as_int();

		if(set->tex_height <= 0)
		{
			set->tex_height = h;
		}

		set->num_tiles_width = set->tex_width / set->tile_width;
		set->num_tiles_height = set->tex_height / set->tile_height;
	}

	return ret;
}

bool j1Map::LoadLayer(pugi::xml_node& node, MapLayer* layer)
{
	bool ret = true;

	layer->name = node.attribute("name").as_string();
	layer->width = node.attribute("width").as_int();
	layer->height = node.attribute("height").as_int();
	LoadProperties(node, layer->properties);
	pugi::xml_node layer_data = node.child("data");

	if(layer_data == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'layer/data' tag.");
		ret = false;
		RELEASE(layer);
	}
	else
	{
		layer->data = new uint[layer->width*layer->height];
		memset(layer->data, 0, layer->width*layer->height);

		int i = 0;
		for(pugi::xml_node tile = layer_data.child("tile"); tile; tile = tile.next_sibling("tile"))
		{
			layer->data[i++] = tile.attribute("gid").as_int(0);
		}
	}

	return ret;
}

// Load a group of properties from a node and fill a list with it
bool j1Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
	bool ret = false;

	pugi::xml_node data = node.child("properties");

	if(data != NULL)
	{
		pugi::xml_node prop;

		for(prop = data.child("property"); prop; prop = prop.next_sibling("property"))
		{
			Properties::Property* p = new Properties::Property();

			p->name = prop.attribute("name").as_string();
			p->value = prop.attribute("value").as_int();

			properties.list_p.push_back(p);
		}
	}

	return ret;
}

bool j1Map::CreateWalkabilityMap(int& width, int& height, uchar** buffer, uint map_id) const
{
	bool ret = false;

	map<uint, MapData*>::const_iterator map_it = maps.find(map_id);
	
	if (map_it == maps.end())
		return ret;

	list<MapLayer*>::const_iterator i = map_it->second->layers.begin();

	while (i != map_it->second->layers.end())
	{

		MapLayer* layer = *i;

		if (layer->properties.Get("Navigation", 0) == 0)
		{
			++i;
			continue;
		}
			
		
		uchar* map = new uchar[layer->width*layer->height];
		memset(map, 1, layer->width*layer->height);

		for (int y = 0; y < map_it->second->height; ++y)
		{
			for (int x = 0; x < map_it->second->width; ++x)
			{
				int i = (y*layer->width) + x;

				int tile_id = layer->Get(x, y);
				TileSet* tileset = (tile_id > 0) ? GetTilesetFromTileId(tile_id, map_id) : NULL;

				if (tileset != NULL)
				{
					map[i] = (tile_id - tileset->firstgid) > 0 ? 0 : 1;
					/*TileType* ts = tileset->GetTileType(tile_id);
					if(ts != NULL)
					{
					map[i] = ts->properties.Get("walkable", 1);
					}*/
				}
			}
		}

		*buffer = map;
		width = map_it->second->width;
		height = map_it->second->height;
		ret = true;
		++i;
	}

	return ret;
}