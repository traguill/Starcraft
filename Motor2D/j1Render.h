#ifndef __j1RENDER_H__
#define __j1RENDER_H__

#include "SDL/include/SDL.h"
#include "p2Point.h"
#include "j1Module.h"
#include <list>

#define CAMERA_TRANSITION_RADIUS 23

struct Sprite;

class j1Render : public j1Module
{
public:

	j1Render();

	// Destructor
	virtual ~j1Render();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Load / Save
	bool Load(pugi::xml_node&);
	bool Save(pugi::xml_node&) const;

	// Utils
	void SetViewPort(const SDL_Rect& rect);
	void ResetViewPort();
	iPoint ScreenToWorld(int x, int y) const;

	// Draw & Blit
	bool Blit(SDL_Texture* texture, int x, int y, const SDL_Rect* section = NULL,uint alpha = 255, float scale = 1.0f, double angle = 0, int pivot_x = INT_MAX, int pivot_y = INT_MAX) const;
	void Blit(Sprite* _sprite, bool priority = false);
	void BlitUI(Sprite _sprite);
	bool DrawQuad(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool filled = true, bool use_camera = true) const;
	bool DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool use_camera = true) const;
	bool DrawCircle(int x1, int y1, int redius, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool use_camera = true, int min = 0, int max = 360) const;

	// Set background color
	void SetBackgroundColor(SDL_Color color);

	//Set a transition to a point
	void SetTransition(int x, int y, bool end_locking = false);

	void MoveAroundQuad(int x, int y, const SDL_Rect& quad);

	void DiscardTransition(); //Stops a transition

	bool GetTransitioning();

	void CursorMovement(float dt); //Only dev_scene should be allowed to use this

private:
	void DoTransition();

	void CheckBoundaries();

public:

	SDL_Renderer*	renderer;
	SDL_Rect		camera;
	SDL_Rect		viewport;
	SDL_Color		background;

	bool			lock_camera = false;
	bool			move_around_quad = false;
private:

	int		camera_speed;

	//Ofsset of the cursor to move the camera
	int		offset_x;
	int		offset_y;
	
	int limit_x;
	int limit_y;

	bool transitioning = false;
	iPoint end_point;

	//Next variable is needed in case the camera can only move around a rectangle
	SDL_Rect quad_boundaries;

	list<Sprite*> blit_sprites;
	list<Sprite*> priority_sprites;
	list<Sprite> ui_sprites;

	bool lock_after_transition = false; //Locks the camera after a transition

	int focus_unit_num = 1;
};

#endif // __j1RENDER_H__