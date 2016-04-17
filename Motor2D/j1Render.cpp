#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Window.h"
#include "j1Render.h"
#include "j1Input.h"
#include "AdvancedMath.h"
#include "j1Map.h"
#include "j1Textures.h"
#include "j1UIManager.h"
#include "UICursor.h"
#include "EntityManager.h"
#include "SceneManager.h"

#define VSYNC true

j1Render::j1Render() : j1Module()
{
	name.append("renderer");
	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
}

// Destructor
j1Render::~j1Render()
{}

// Called before render is available
bool j1Render::Awake(pugi::xml_node& config)
{
	LOG("Create SDL rendering context");
	bool ret = true;
	// load flags
	Uint32 flags = SDL_RENDERER_ACCELERATED;

	if(config.child("vsync").attribute("value").as_bool(true) == true)
	{
		flags |= SDL_RENDERER_PRESENTVSYNC;
		LOG("Using vsync");
	}

	renderer = SDL_CreateRenderer(App->win->window, -1, flags);

	if(renderer == NULL)
	{
		LOG("Could not create the renderer! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		camera.w = App->win->screen_surface->w;
		camera.h = App->win->screen_surface->h;
		camera.x = 0;
		camera.y = 0;

		camera_speed = config.child("camera").attribute("speed").as_int(2);
		offset_x = config.child("camera").attribute("offset_x").as_int(20);
		offset_y = config.child("camera").attribute("offset_y").as_int(15);
		limit_x = config.child("camera").attribute("limit_x").as_int(-2816);
		limit_y = config.child("camera").attribute("limit_y").as_int(-3072);
	}

	return ret;
}

// Called before the first frame
bool j1Render::Start()
{
	LOG("render start");
	// back background
	SDL_RenderGetViewport(renderer, &viewport);
	return true;
}

// Called each loop iteration
bool j1Render::PreUpdate()
{
	SDL_RenderClear(renderer);
	return true;
}

bool j1Render::Update(float dt)
{
	if (!lock_camera)
	{
		if (transitioning == true)
			DoTransition();
		else

			if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT)
			{
				if (App->entity->selected_units.size() > 0)
				{
					camera.x = -App->entity->selected_units.front()->GetPosition().x + (camera.w / 2);
					camera.y = -App->entity->selected_units.front()->GetPosition().y + (camera.h / 2);
				}
			}
			else if (App->scene_manager->in_game == true)
				CursorMovement(dt);
	}



	//Sort Sprites and blit
	blit_sprites.sort([](const Sprite* a, const Sprite* b) { return a->position.y < b->position.y; });

	list<Sprite*>::iterator i = blit_sprites.begin();

	while (i != blit_sprites.end())
	{
		Blit((*i)->texture, (*i)->position.x, (*i)->position.y, &(*i)->rect, (*i)->alpha);
		++i;
	}



	return true;
}

bool j1Render::PostUpdate()
{
	SDL_SetRenderDrawColor(renderer, background.r, background.g, background.g, background.a);
	SDL_RenderPresent(renderer);

	blit_sprites.clear();

	return true;
}

// Called before quitting
bool j1Render::CleanUp()
{
	LOG("Destroying SDL render");
	SDL_DestroyRenderer(renderer);
	return true;
}

// Load Game State
bool j1Render::Load(pugi::xml_node& data)
{
	camera.x = data.child("camera").attribute("x").as_int();
	camera.y = data.child("camera").attribute("y").as_int();

	return true;
}

// Save Game State
bool j1Render::Save(pugi::xml_node& data) const
{
	pugi::xml_node cam = data.append_child("camera");

	cam.append_attribute("x") = camera.x;
	cam.append_attribute("y") = camera.y;

	return true;
}

void j1Render::SetBackgroundColor(SDL_Color color)
{
	background = color;
}

void j1Render::SetViewPort(const SDL_Rect& rect)
{
	SDL_RenderSetViewport(renderer, &rect);
}

void j1Render::ResetViewPort()
{
	SDL_RenderSetViewport(renderer, &viewport);
}

iPoint j1Render::ScreenToWorld(int x, int y) const
{
	iPoint ret;
	int scale = App->win->GetScale();

	ret.x = (x - camera.x / scale);
	ret.y = (y - camera.y / scale);

	return ret;
}

// Blit to screen

void j1Render::Blit(Sprite* _sprite)
{
	if (_sprite != NULL)
		blit_sprites.push_back(_sprite);
}
bool j1Render::Blit(SDL_Texture* texture, int x, int y, const SDL_Rect* section,uint alpha, float speed, double angle, int pivot_x, int pivot_y) const
{
	bool ret = true;
	uint scale = App->win->GetScale();

	SDL_Rect rect;
	rect.x = (int)(camera.x * speed) + x * scale;
	rect.y = (int)(camera.y * speed) + y * scale;

	if(section != NULL)
	{
		rect.w = section->w;
		rect.h = section->h;
	}
	else
	{
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	}

	rect.w *= scale;
	rect.h *= scale;

	SDL_Point* p = NULL;
	SDL_Point pivot;

	if(pivot_x != INT_MAX && pivot_y != INT_MAX)
	{
		pivot.x = pivot_x;
		pivot.y = pivot_y;
		p = &pivot;
	}

	if (alpha != 255 && alpha < 255)
	{
		SDL_SetTextureAlphaMod(texture, alpha);
	}

	if(SDL_RenderCopyEx(renderer, texture, section, &rect, angle, p, SDL_FLIP_NONE) != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		ret = false;
	}

	//Reset the texture to the original alpha mode
	if (alpha != 255)
	{
		SDL_SetTextureAlphaMod(texture, 255);
	}

	return ret;
}

bool j1Render::DrawQuad(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
	bool ret = true;
	uint scale = App->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_Rect rec(rect);
	if(use_camera)
	{
		rec.x = (int)(camera.x + rect.x * scale);
		rec.y = (int)(camera.y + rect.y * scale);
		rec.w *= scale;
		rec.h *= scale;
	}

	int result = (filled) ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderDrawRect(renderer, &rec);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool j1Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	uint scale = App->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;

	if(use_camera)
		result = SDL_RenderDrawLine(renderer, camera.x + x1 * scale, camera.y + y1 * scale, camera.x + x2 * scale, camera.y + y2 * scale);
	else
		result = SDL_RenderDrawLine(renderer, x1 * scale, y1 * scale, x2 * scale, y2 * scale);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool j1Render::DrawCircle(int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	uint scale = App->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;
	SDL_Point points[360];

	float factor = (float)M_PI / 180.0f;

	if (use_camera)
	{
		for (uint i = 0; i < 360; ++i)
		{
			points[i].x = (int)(camera.x + x + radius * cos(i * factor) * scale);
			points[i].y = (int)(camera.y + y + radius * sin(i * factor) * scale);
		}
	}

	result = SDL_RenderDrawPoints(renderer, points, 360);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

void j1Render::CursorMovement(float dt)
{
	int mouse_x, mouse_y;
	App->input->GetMousePosition(mouse_x, mouse_y);

	// doesnt enter none if
	if (App->ui->cursor_state != ON_FRIENDLY && App->ui->cursor_state != ON_ENEMY)
	App->ui->cursor_state = STANDARD;

	//Move camera LEFT
	if (mouse_x  < offset_x)
	{
		camera.x += dt * camera_speed;
		App->ui->cursor_state = TO_LEFT;
	}
	//Move camera RIGHT
	if (mouse_x > camera.w - offset_x)
	{
		camera.x -= dt * camera_speed;
		App->ui->cursor_state = TO_RIGHT;
	}
	//Move camera UP
	if (mouse_y < offset_y)
	{
		camera.y += dt * camera_speed;
		App->ui->cursor_state = UP;
	}
	//Move camera DOWN
	if (mouse_y > camera.h - offset_y)
	{
		camera.y -= dt * camera_speed;
		App->ui->cursor_state = DOWN;
	}	


	//Limits
	if (camera.x > 0)			camera.x = 0;
	if (camera.x < limit_x)		camera.x = limit_x;
	if (camera.y > 0)			camera.y = 0;
	if (camera.y < limit_y)		camera.y = limit_y;
}

void j1Render::SetTransition(int x, int y,bool end_locking)
{
	transitioning = true;
	end_point.x = x;
	end_point.y = y;

	//Limits
	if (end_point.x > 0)			end_point.x = 0;
	if (end_point.x < limit_x)		end_point.x = limit_x;
	if (end_point.y > 0)			end_point.y = 0;
	if (end_point.y < limit_y)		end_point.y = limit_y;

	if (end_locking)
		lock_after_transition = true;
}

void j1Render::DoTransition()
{
	camera.x = lerp(camera.x, end_point.x, 0.05f);
	camera.y = lerp(camera.y, end_point.y, 0.05f);

	if (end_point.DistanceTo(iPoint(camera.x, camera.y)) < CAMERA_TRANSITION_RADIUS)
	{
		transitioning = false;
		if (lock_after_transition == true)
		{
			lock_camera = true;
			lock_after_transition = false;
		}
			
	}
		
}