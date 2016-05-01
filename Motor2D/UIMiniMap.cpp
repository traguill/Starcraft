#include "UIEntity.h"
#include "UIMiniMap.h"
#include "j1App.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "j1UIManager.h"
#include "p2Log.h"
#include "j1Input.h"
#include "j1Window.h"
#include "EntityManager.h"

UIMiniMap::UIMiniMap() : UIEntity()
{
	type = MINI_MAP;
	map_state = IDLE_MAP;

	iPoint cam(App->render->camera.x, App->render->camera.y);
	GetScreenPos(x_mouse, y_mouse);

	div_x = 1728 / 130;
	div_y = 4096 / 130;

	uint _w, _h;
	App->win->GetWindowSize(_w, _h);

	white_rec.w = _w / div_x;
	white_rec.h = _h / div_y;

	white_rec.x = x_mouse - (white_rec.w / 2);
	white_rec.y = y_mouse - (white_rec.h / 2);

}


UIMiniMap::UIMiniMap(SDL_Rect position, SDL_Rect section_drawn) : UIEntity()
{


	rect = position;
	draw_section = section_drawn;
	type = MINI_MAP;
	map_state = IDLE_MAP;

	iPoint cam(App->render->camera.x, App->render->camera.y);
	GetScreenPos(x_mouse, y_mouse);

	div_x = 1728 / 130;
	div_y = 3988 / 130;

	uint _w, _h;
	App->win->GetWindowSize(_w, _h);

	white_rec.w = _w / div_x;
	white_rec.h = _h / div_y;

	white_rec.x = x_mouse - (white_rec.w / 2);
	white_rec.y = y_mouse - (white_rec.h / 2);


}

UIMiniMap::~UIMiniMap()
{}

bool UIMiniMap::Update(float dt)
{
	bool ret = true;

	int x, y;

	GetScreenPos(x, y);

	iPoint cam(App->render->camera.x, App->render->camera.y);

	App->render->Blit(App->ui->GetAtlas(), x - cam.x, y - cam.y, &draw_section);

	UpdateUnitsMiniMap();
	UpdateRect();

	return ret;
}


iPoint UIMiniMap::GetRectLocalPos()
{
	iPoint tmp;

	tmp.x = (x_mouse - (white_rec.w / 2) - rect.x);
	tmp.y = (y_mouse - (white_rec.h / 2) - rect.y);

	return (tmp);
}

void UIMiniMap::UpdateRect()
{
	int x, y;

	GetScreenPos(x, y);

	iPoint cam(App->render->camera.x, App->render->camera.y);

	GetState();

	white_rec.x = x_mouse - (white_rec.w / 2);
	white_rec.y = y_mouse - (white_rec.h / 2);

	if (map_state == PRESSED_MAP)
	{
		App->input->GetMousePosition(x_mouse, y_mouse);

		iPoint loc_pos = GetRectLocalPos();

		//Limits for white rectancle
		if (loc_pos.x > (rect.w - white_rec.w))
		{
			loc_pos.x = rect.w - white_rec.w;
			x_mouse = loc_pos.x + rect.x + (white_rec.w / 2);
		}

		if (loc_pos.y > (rect.h - white_rec.h))
		{
			loc_pos.y = rect.h - white_rec.h;
			y_mouse = loc_pos.y + rect.y + (white_rec.h / 2);
		}

		if (loc_pos.x < 0)
		{
			loc_pos.x = 0;
			x_mouse = loc_pos.x + rect.x + (white_rec.w / 2);
		}

		if (loc_pos.y < 0)
		{
			loc_pos.y = 0;
			y_mouse = loc_pos.y + rect.y + (white_rec.h / 2);
		}



		int _x = loc_pos.x * div_x;
		int _y = loc_pos.y * div_y;

		App->render->SetTransition(-_x, -_y);
		click_transition = true;

		white_rec.x = loc_pos.x + rect.x;
		white_rec.y = loc_pos.y + rect.y;

		map_state = IDLE_MAP;
	}

	else
	{
		if (!click_transition)
		{
			iPoint rec_cam = { (-cam.x / div_x) + rect.x, (-cam.y / div_y) + rect.y };

			if (rec_cam.x != white_rec.x && rec_cam.y != white_rec.y)
			{
				white_rec.x = rec_cam.x;
				white_rec.y = rec_cam.y;
			}
		}

		else if (!App->render->GetTransitioning() && click_transition)
		{
			click_transition = false;
		}
	}


	SDL_Rect drawn_rec = white_rec;

	drawn_rec.x = drawn_rec.x - cam.x;
	drawn_rec.y = drawn_rec.y - cam.y;

	App->render->DrawQuad(drawn_rec, 255, 255, 255, 255, false, true);

}

void UIMiniMap::UpdateUnitsMiniMap()
{
	iPoint cam(App->render->camera.x, App->render->camera.y);

	list<Unit*>::iterator it_uf = App->entity->friendly_units.begin();

	while (it_uf != App->entity->friendly_units.end())
	{
		int x = (*it_uf)->GetPosition().x;
		int y = (*it_uf)->GetPosition().y;

		App->render->DrawQuad({ (x / div_x) + rect.x - cam.x, (y / div_y) + rect.y - cam.y, 2, 2 }, 0, 0, 255, 255, true, true);

		it_uf++;
	}

	list<Unit*>::iterator it_ue = App->entity->enemy_units.begin();

	while (it_ue != App->entity->enemy_units.end())
	{
		int x = (*it_ue)->GetPosition().x;
		int y = (*it_ue)->GetPosition().y;

		App->render->DrawQuad({ (x / div_x) + rect.x - cam.x, (y / div_y) + rect.y - cam.y, 2, 2 }, 255, 0, 0, 255, true, true);

		it_ue++;
	}


}

bool UIMiniMap::CleanUp()
{
	bool ret = true;


	return ret;
}

void UIMiniMap::GetState()
{
	if (gui_event == MOUSE_ENTER)
	{
		map_state = HOVER_MAP;
	}

	if (gui_event == MOUSE_EXIT)
	{
		map_state = IDLE_MAP;
	}

	if (gui_event == MOUSE_BUTTON_RIGHT_DOWN)
	{
		map_state = PRESSED_MAP;
	}

}