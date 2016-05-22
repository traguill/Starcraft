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
#include "GameScene.h"

UIMiniMap::UIMiniMap() : UIEntity()
{
	type = MINI_MAP;
	map_state = IDLE_MAP;
	last_state = map_state;

	iPoint cam(App->render->camera.x, App->render->camera.y);

	div_x = 1728 / 130;
	div_y = 4096 / 130;

	uint _w, _h;
	App->win->GetWindowSize(_w, _h);

	white_rec.w = _w / div_x;
	white_rec.h = _h / div_y;
	
	init_pos.x = rect.x;
	init_pos.y = rect.y;

	offset.x = 50;
	offset.y = 50;


}


UIMiniMap::UIMiniMap(SDL_Rect position, SDL_Rect section_drawn, iPoint original_map_size) : UIEntity()
{
	rect = position;

	type = MINI_MAP;
	map_state = IDLE_MAP;
	last_state = map_state;

	iPoint cam(App->render->camera.x, App->render->camera.y);

	//div_x = 1728 / section_drawn.w;
	//div_y = 3988 / section_drawn.h;

	div_x = original_map_size.x / section_drawn.w;
	div_y = original_map_size.y / section_drawn.h;

	uint _w, _h;
	App->win->GetWindowSize(_w, _h);

	white_rec.w = _w / div_x;
	white_rec.h = _h / div_y;

	init_pos.x = rect.x;
	init_pos.y = rect.y;

	offset.x = 50;
	offset.y = 50;

	isFocus = false;

	ui_sprite.texture = App->ui->GetAtlas();
	ui_sprite.position = init_pos;
	ui_sprite.rect = section_drawn;

}

UIMiniMap::~UIMiniMap()
{}

bool UIMiniMap::Update(float dt)
{
	bool ret = true;

	iPoint cam(App->render->camera.x, App->render->camera.y);

	ui_sprite.position.x = init_pos.x - cam.x;
	ui_sprite.position.y = init_pos.y - cam.y + 3;

	App->render->BlitUI(&ui_sprite);
	
	GetState();
	UpdateUnitsMiniMap();
	if (App->game_scene->GetTutorialState() && !App->game_scene->GetFinishedGame())
		UpdateRect();

	return ret;
}


iPoint UIMiniMap::WhiteRectUpdatedPos()
{
	iPoint tmp(App->render->camera.x, App->render->camera.y);
	
	tmp.x = (-tmp.x / div_x) + rect.x;
	tmp.y = (-tmp.y / div_y) + rect.y;

	return (tmp);
}

void UIMiniMap::UpdateRect()
{

	if (map_state == PRESSED_MAP)
	{
		
		App->input->GetMouseWorld(white_rec.x, white_rec.y);
		white_rec.x = white_rec.x - (white_rec.w / 2);
		white_rec.y = white_rec.y - (white_rec.h / 2);

		iPoint new_pos(-((white_rec.x - rect.x)*div_x), -((white_rec.y - rect.y)*div_y));

		int x, y;
		App->input->GetMouseMotion(x, y);

		if ((x != 0 || y != 0) && last_state != CONTINUE_PRESS_MAP)
		{
			map_state = CONTINUE_PRESS_MAP;
		}
	
		else
		{
			App->render->camera.x = new_pos.x;
			App->render->camera.y = new_pos.y;
			
		}

		last_state = PRESSED_MAP;
	}

	if (map_state == CONTINUE_PRESS_MAP)
	{
		int mouse_x, mouse_y;
		App->input->GetMouseWorld(mouse_x, mouse_y);
		white_rec.x = mouse_x - (white_rec.w / 2);
		white_rec.y = mouse_y - (white_rec.h / 2);

		//Limits for the minimap
		if (white_rec.x < rect.x)
		{
			white_rec.x = rect.x;
		}

		if (white_rec.y < rect.y)
		{
			white_rec.y = rect.y;
		}

		if (white_rec.x - rect.x > rect.w - white_rec.w)
		{
			white_rec.x = (rect.w - white_rec.w) + rect.x;
		}

		if (white_rec.y - rect.y > rect.h - (white_rec.h - 4))
		{
			white_rec.y = (rect.h - (white_rec.h - 4)) + rect.y;
		}

		iPoint new_pos(-((white_rec.x - rect.x)*div_x), -((white_rec.y - rect.y)*div_y));
		
		App->render->camera.x = new_pos.x;
		App->render->camera.y = new_pos.y;

		rect.x = init_pos.x - new_pos.x;
		rect.y = init_pos.y - new_pos.y;

	}

	draw_pos = WhiteRectUpdatedPos();
	App->render->DrawQuad({ draw_pos.x, draw_pos.y, white_rec.w, white_rec.h }, 255, 255, 255, 255, false, true);
}

void UIMiniMap::UpdateUnitsMiniMap()
{
	iPoint cam(App->render->camera.x, App->render->camera.y);

	list<Unit*>::iterator it_uf = App->entity->friendly_units.begin();

	while (it_uf != App->entity->friendly_units.end())
	{
		int x = (*it_uf)->GetPosition().x;
		int y = (*it_uf)->GetPosition().y;

		App->render->DrawQuad({ (x / div_x) + rect.x, (y / div_y) + rect.y, 2, 2 }, 0, 0, 255, 255, true, true);

		it_uf++;
	}

	list<Unit*>::iterator it_ue = App->entity->enemy_units.begin();

	while (it_ue != App->entity->enemy_units.end())
	{
		int x = (*it_ue)->GetPosition().x;
		int y = (*it_ue)->GetPosition().y;

		App->render->DrawQuad({ (x / div_x) + rect.x, (y / div_y) + rect.y, 2, 2 }, 255, 0, 0, 255, true, true);

		it_ue++;
	}


	list<iPoint>::iterator it_b = App->game_scene->bomb_pos.begin();

	while (it_b != App->game_scene->bomb_pos.end())
	{
		int x = it_b->x;
		int y = it_b->y;

		App->render->DrawQuad({ (x / div_x) + rect.x, (y / div_y) + rect.y, 2, 2 }, 255, 255, 0, 255, true, true);

		it_b++;
	}

	if (App->game_scene->bomb_pos.size() == 0)
	{
		int x = App->game_scene->bomb_zone.x;
		int y = App->game_scene->bomb_zone.y;

		App->render->DrawQuad({ (x / div_x) + rect.x, (y / div_y) + rect.y, 2, 2 }, 255, 255, 0, 255, true, true);

	}


}

bool UIMiniMap::CleanUp()
{
	bool ret = true;


	return ret;
}

void UIMiniMap::GetState()
{

	if (last_state == PRESSED_MAP && App->input->GetMouseButtonDown(1) != KEY_UP)
	{
		map_state = CONTINUE_PRESS_MAP;
		last_state = CONTINUE_PRESS_MAP;
	}

	if (gui_event == MOUSE_ENTER && map_state != CONTINUE_PRESS_MAP)
	{
		map_state = HOVER_MAP;
		last_state = HOVER_MAP;
	}
	
	if (gui_event == MOUSE_BUTTON_RIGHT_DOWN && map_state != CONTINUE_PRESS_MAP)
	{
			map_state = PRESSED_MAP;
			last_state = PRESSED_MAP;
	}

	if (gui_event == MOUSE_EXIT && map_state != CONTINUE_PRESS_MAP)
	{
		map_state = IDLE_MAP;
		last_state = IDLE_MAP;
	}

	int of_x, of_y;

	App->input->GetMouseWorld(of_x, of_y);

	if (gui_event == MOUSE_EXIT && map_state == CONTINUE_PRESS_MAP && ((of_x > rect.x + rect.w + offset.x) || (of_y < rect.y - offset.y)))
	{
		map_state = PRESSED_MAP;
		last_state = PRESSED_MAP;
	}

	if (App->input->GetMouseButtonDown(1) == KEY_UP)
	{
		map_state = IDLE_MAP;
	}
	

}