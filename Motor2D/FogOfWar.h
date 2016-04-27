#ifndef __FOG_WAR__
#define __FOG_WAR__

#include "j1Module.h"
#include <vector>

class Fog_Map
{
public:
	Fog_Map(int w, int h);
	~Fog_Map();

	//Draw a circle onto the map
	void DrawCircle(int _x, int _y, int radius, bool visible = true);

	//Set all tiles to a value
	void SetAll(bool visible);

	//Copy this map's tiles to another map
	void CopyTo(Fog_Map* output);


	//Returns if a certain tile is visible or not
	bool isVisible(int x, int y);

	//Number of width tiles
	uint GetWidth() { return w; }
	//Number of height tiles
	uint GetHeight() { return h; }

	//Soften the edges of a certain section of the map
	void SoftenSection(int x1, int y1, int x2, int y2, float fadeRatio = 1.5f);

	//Alpha the non-visible tiles will have
	uint maxAlpha = 255;

	uint** map;

	//Defines if this map will be drawn
	bool draw = true;
private:
	uint w = 0;
	uint h = 0;
};

class FogOfWar : public j1Module
{
public:

	FogOfWar();

	// Destructor
	~FogOfWar();

	bool CleanUp();

	//Create & Initialize the fog maps
	bool SetUp(uint graphicalW, uint graphicalH, uint mapW = 100, uint mapH = 100, uint nMaps = 1);
	//Delete the fog maps
	void EraseMaps();

	//Draw the fog
	void Draw();

	//Draw a circle on a certain Fog Map. Leave on -1 to draw on all
	void DrawCircle(int x, int y, uint radius, bool visible = true, int map = -1);
	//Reset a Fog Map to not visible. Leave on -1 to reset all.
	void ClearMap(int map = -1);
	//Copy a certain Fog map tiles values to another.
	bool Copy(uint from, uint to);

	//Return if a certain tile is visible or not
	bool IsVisible(int x, int y);

private:
	//Create a new FogMap
	int CreateMap(int w, int h, int maxAlpha = 255);

public:
	//List of Fog maps
	vector<Fog_Map*> maps;
private:
	//Declares if the module has been SetUp and it's usable or not.
	bool ready = false;
	//Width & Height of each tile in pixels
	int tileW, tileH = 0;

};

#endif 