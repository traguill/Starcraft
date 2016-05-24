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

	white_rect.rect.x = 875;
	white_rect.rect.y = 345;
	white_rect.rect.w = _w / div_x;
	white_rect.rect.h = _h / div_y;
	
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


	white_rect.rect.x = 875;
	white_rect.rect.y = 345;
	white_rect.rect.w = _w / div_x;
	white_rect.rect.h = _h / div_y;

	init_pos.x = rect.x;
	init_pos.y = rect.y;

	offset.x = 50;
	offset.y = 50;

	isFocus = false;

	ui_sprite.texture = white_rect.texture = App->ui->GetAtlas();
	ui_sprite.position = init_pos;
	ui_sprite.rect = section_drawn;

}

UIMiniMap::~UIMiniMap()
{
	ui_sprite.texture = NULL;
}

bool UIMiniMap::PreUpdate()
{
	if (App->game_scene->GetTutorialState() && !App->game_scene->GetFinishedGame())
		UpdateRect();

	return true;
}

bool UIMiniMap::Update(float dt)
{
	bool ret = true;

	iPoint cam(App->render->camera.x, App->render->camera.y);

	rect.x = init_pos.x - cam.x;
	rect.y = init_pos.y - cam.y + 3;

	ui_sprite.position.x = rect.x;
	ui_sprite.position.y = rect.y;

	App->render->BlitUI(ui_sprite);
	
	GetState();
	UpdateUnitsMiniMap();
	

	white_rect.position = WhiteRectUpdatedPos();

	App->render->BlitUI(white_rect);

	return ret;
}

void UIMiniMap::EnableMinimap(bool enable)
{
	enable_minimap = enable;
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
	if (enable_minimap)
	{
		if (map_state == PRESSED_MAP)
		{

			App->input->GetMouseWorld(white_rect.position.x, white_rect.position.y);
			white_rect.position.x = white_rect.position.x - (white_rect.rect.w / 2);
			white_rect.position.y = white_rect.position.y - (white_rect.rect.h / 2);

			iPoint new_pos(-((white_rect.position.x - ui_sprite.position.x)*div_x), -((white_rect.position.y - ui_sprite.position.y)*div_y));

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
			white_rect.position.x = mouse_x - (white_rect.rect.w / 2);
			white_rect.position.y = mouse_y - (white_rect.rect.h / 2);

			//Limits for the minimap
			if (white_rect.position.x < ui_sprite.position.x)
			{
				white_rect.position.x = ui_sprite.position.x;
			}

			if (white_rect.position.y < ui_sprite.position.y)
			{
				white_rect.position.y = ui_sprite.position.y;
			}

			if (white_rect.position.x - ui_sprite.position.x > ui_sprite.rect.w - white_rect.rect.w)
			{
				white_rect.position.x = (ui_sprite.rect.w - white_rect.rect.w) + ui_sprite.position.x;
			}

			if (white_rect.position.y - ui_sprite.position.y > ui_sprite.rect.h - (white_rect.rect.h - 4))
			{
				white_rect.position.y = (ui_sprite.rect.h - (white_rect.rect.h - 4)) + ui_sprite.position.y;
			}

			iPoint new_pos(-((white_rect.position.x - ui_sprite.position.x)*div_x), -((white_rect.position.y - ui_sprite.position.y)*div_y));

			App->render->camera.x = new_pos.x;
			App->render->camera.y = new_pos.y;

			ui_sprite.position.x = init_pos.x - new_pos.x;
			ui_sprite.position.y = init_pos.y - new_pos.y;

		}
	}

	white_rect.position = WhiteRectUpdatedPos();
	
}

void UIMiniMap::UpdateUnitsMiniMap()
{
	iPoint cam(App->render->camera.x, App->render->camera.y);

	Sprite point_friend, point_enemy, point_objective;
	point_friend.texture = point_enemy.texture = point_objective.texture = App->ui->GetAtlas();
	point_friend.rect.w = point_friend.rect.h = point_enemy.rect.w = point_enemy.rect.h = point_objective.rect.w = point_objective.rect.h = 1;

	point_friend.rect.x = 871;
	point_friend.rect.y = 365;

	point_enemy.rect.x = 870;
	point_enemy.rect.y = 365;

	point_objective.rect.x = 872;
	point_objective.rect.y = 365;

	list<Unit*>::iterator it_uf = App->entity->friendly_units.begin();

	while (it_uf != App->entity->friendly_units.end())
	{
		point_friend.position.x = (*it_uf)->GetPosition().x / div_x + ui_sprite.position.x;
		point_friend.position.y = (*it_uf)->GetPosition().y / div_y + ui_sprite.position.y;

		App->render->BlitUI(point_friend);
		
		it_uf++;
	}

	list<Unit*>::iterator it_ue = App->entity->enemy_units.begin();

	while (it_ue != App->entity->enemy_units.end())
	{
		point_enemy.position.x = (*it_ue)->GetPosition().x / div_x + ui_sprite.position.x;
		point_enemy.position.y = (*it_ue)->GetPosition().y / div_y + ui_sprite.position.y;

		App->render->BlitUI(point_enemy);

		it_ue++;
	}


	list<iPoint>::iterator it_b = App->game_scene->bomb_pos.begin();

	while (it_b != App->game_scene->bomb_pos.end())
	{
		point_objective.position.x = it_b->x / div_x + ui_sprite.position.x;
		point_objective.position.y = it_b->y / div_y + ui_sprite.position.y;

		App->render->BlitUI(point_objective);

		it_b++;
	}

	if (App->game_scene->bomb_pos.size() == 0)
	{
		point_objective.position.x = App->game_scene->bomb_zone.x / div_x + ui_sprite.position.x;
		point_objective.position.y = App->game_scene->bomb_zone.y / div_y + ui_sprite.position.y;

		App->render->BlitUI(point_objective);
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


void UIMiniMap::GetScreenPos(int &x, int &y)const
{
	x = y = 0;

	x += rect.x;
	y += rect.y;
}

void UIMiniMap::GetLocalPos(int &x, int &y)const
{
	x = rect.x;
	y = rect.y;
}

SDL_Rect UIMiniMap::GetScreenRect()const
{
	SDL_Rect ret = rect;
	GetScreenPos(ret.x, ret.y);

	return ret;
}

SDL_Rect UIMiniMap::GetLocalRect()const
{
	return rect;;
}

void UIMiniMap::SetLocalPos(int x, int y)
{
	ui_sprite.position.x = ui_sprite.rect.x = rect.x = x;
	ui_sprite.position.y = ui_sprite.rect.y = rect.y = y;
}