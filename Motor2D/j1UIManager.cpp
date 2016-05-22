#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1FileSystem.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Fonts.h"
#include "j1Input.h"
#include "j1UIManager.h"
#include "UILabel.h"
#include "UIImage.h"
#include "UIButton.h"
#include "UIProgressBar.h"
#include "j1Window.h"
#include "UIInputBox.h"
#include "UICursor.h"
#include "EntityManager.h"
#include "SceneManager.h"
#include <stdio.h>
#include "GameScene.h"
#include "MenuScene.h"
#include "UIMiniMap.h"
#include "Ghost.h"

j1UIManager::j1UIManager() : j1Module()
{
	name.append("gui");
	debug = false;
}

// Destructor
j1UIManager::~j1UIManager()
{}

// Called before render is available
bool j1UIManager::Awake(pugi::xml_node& conf)
{
	LOG("Loading UIManager atlas");
	bool ret = true;

	atlas_file_name = conf.child("atlas").attribute("file").as_string("");

	ui_file_path = conf.child("gui_data").attribute("value").as_string("");

	return ret;
}

// Called before the first frame
bool j1UIManager::Start()
{
	bool ret = true;

	atlas = App->tex->Load(atlas_file_name.data());
	ghost_tex = App->tex->Load("gui/ghost_ui.png");

	SDL_ShowCursor(SDL_DISABLE);

	cursor_state = STANDARD;

	vector<SDL_Rect> sections;
	sections.push_back({ 1, 62, 20, 21 });
	sections.push_back({ 22, 62, 20, 21 });
	sections.push_back({ 43, 62, 20, 21 });
	sections.push_back({ 64, 62, 20, 21 });
	sections.push_back({ 85, 62, 20, 21 });
	cursor = App->ui->CreateCursor(sections, 0.08);

	return ret;
}

bool j1UIManager::LoadUiInfo()
{
	bool ret = true;

	pugi::xml_document	ui_file;
	pugi::xml_node		ui_elements;

	char* buf;
	int size = App->fs->Load(ui_file_path.c_str(), &buf);
	pugi::xml_parse_result result = ui_file.load_buffer(buf, size);
	
	delete[] buf;
	buf = NULL;

	if (result == NULL)
	{
		LOG("Could not load xml file %s. PUGI error: %s", ui_file_path.c_str(), result.description());
		return false;
	}
	else
	{
		LOG("GUI ELEMENTS");
		ui_elements = ui_file.child("gui_elements");
	}

	pugi::xml_node element;
	for (element = ui_elements.child("gui"); element; element = element.next_sibling("gui"))
	{
		string progress_bar = "PROGRESS_BAR";
		string image = "IMAGE";
		string gui_type = element.attribute("type").as_string();

		//Progressbar
		if (gui_type == progress_bar)
		{
			UIProgressBar* bar = new UIProgressBar();

			//Fill data here
			bar->bar_tex = App->tex->Load(element.child("texture").attribute("value").as_string());
			bar->GetSprite()->texture = bar->bar_tex;

			bar->SetLocalPos(element.child("local_pos").attribute("x").as_int(), element.child("local_pos").attribute("y").as_int());

			int width = element.child("size").attribute("width").as_int();
			int height = element.child("size").attribute("height").as_int();

			bar->full_bar_section = { element.child("full").attribute("x").as_int(), element.child("full").attribute("y").as_int(), width, height };
			bar->middle_bar_section = { element.child("middle").attribute("x").as_int(), element.child("middle").attribute("y").as_int(), width, height };
			bar->low_bar_section = { element.child("low").attribute("x").as_int(), element.child("low").attribute("y").as_int(), width, height };
			bar->empty_bar_section = { element.child("empty").attribute("x").as_int(), element.child("empty").attribute("y").as_int(), width, height };

			string bar_type = element.child("bar_type").attribute("value").as_string();

			//Types of bars
			if (bar_type == "HEALTH")
				bar->bar_type = HEALTH;

			if (bar_type == "MANA")
				bar->bar_type = MANA;

			gui_database.insert(pair<string, UIEntity*>(element.attribute("key").as_string(), bar));
		}

		else if (gui_type == image)
		{
			SDL_Rect section = { element.child("section").attribute("x").as_int(), element.child("section").attribute("y").as_int(),
								 element.child("section").attribute("w").as_int(), element.child("section").attribute("h").as_int() };

			int x = ui_elements.child("mini_wireframe_pos").attribute("x").as_int();
			int y = ui_elements.child("mini_wireframe_pos").attribute("y").as_int();
			
			UIImage* image = new UIImage(section, x, y);

			image->SetVisible(false);

			gui_database.insert(pair<string, UIEntity*>(element.attribute("key").as_string(), image));
		}
	}

	mw_width = ui_elements.child("mini_wireframe_width").attribute("value").as_int();
	mw_height = ui_elements.child("mini_wireframe_height").attribute("value").as_int();

	return ret;
}
// Update all UIManagers
bool j1UIManager::PreUpdate()
{
	list<UIEntity*>::iterator i = gui_elements.begin();
	while (i != gui_elements.end())
	{
		if ((*i)->IsVisible() == true)
		{
			(*i)->PreUpdate();
		}
		++i;
	}
	return true;
}

// Called after all Updates
bool j1UIManager::PostUpdate()
{

	RectangleSelection();
	return true;
}

bool j1UIManager::Update(float dt)
{
	bool ret = true;

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_UP)
		debug = !debug;

	GetMouseInput();

	//UI Focus disabled for now
	/*if (App->input->GetKey(SDL_SCANCODE_TAB) == KEY_DOWN) 
	{
		SetNextFocus();
	}*/

	UpdateAnimation(dt);

	//Draw lifes & mana
	DrawLifeMana();

	

	list<UIEntity*>::iterator i = gui_elements.begin();
	while (i != gui_elements.end() && ret == true)
	{
		if ((*i)->IsVisible() == true)
		{
			ret = (*i)->Update(dt);
			(*i)->GUIEvents();
			if (debug)
				(*i)->Debug();

		}
		++i;
	}

	if (App->entity->selected_units.size() > 1)
		ShowMiniWireframes();
	
	else if (App->entity->selected_units.size() == 1)
		ShowIndividualWireframe();


	if (App->scene_manager->in_game == true && App->entity->SNIPPER_MODE)
	{
		//snipper_ui->SetVisible(true);
		
		iPoint pos;
		SDL_Rect rec;
		list<Unit*>::iterator it = App->entity->selected_units.begin();
		for (it; it != App->entity->selected_units.end(); it++)
		{
			if ((*it)->GetType() == GHOST) 
			{
				Ghost* ghost = (Ghost*)(*it);
				if (ghost->GetSnipping() == true)
				{
					rec = (*it)->current_animation->getCurrentFrame();
					pos = (*it)->GetDrawPosition();
				}
			}
		}
		rec = { rec.x - 5, rec.y - 5, rec.w + 10, rec.h + 15 };
		App->render->Blit(ghost_tex, pos.x - 5, pos.y - 5, &rec);

		App->game_scene->snipper_ui->SetVisible(true);

	}

	else if (App->scene_manager->in_game == true && !App->entity->SNIPPER_MODE)
	{
		App->game_scene->snipper_ui->SetVisible(false);
	}

	cursor->Update(dt);

	if (App->game_scene->GamePaused() && App->scene_manager->in_game)
	{
		if (App->game_scene->active_timer == false)
		{
			App->game_scene->pause_timer.Start();
			App->game_scene->active_timer = true;
		}

		if (App->game_scene->pause_timer.ReadSec() <= 0.8)
		{
			App->render->DrawQuad(SDL_Rect{ -App->render->camera.x + 1, -App->render->camera.y + 1, App->render->camera.w - 2, App->render->camera.h - 2 }, 255, 0, 0, 255, false, true);
			App->render->DrawQuad(SDL_Rect{ -App->render->camera.x + 2, -App->render->camera.y + 2, App->render->camera.w - 4, App->render->camera.h - 4 }, 255, 0, 0, 255, false, true);
			App->render->DrawQuad(SDL_Rect{ -App->render->camera.x + 3, -App->render->camera.y + 3, App->render->camera.w - 6, App->render->camera.h - 6 }, 255, 0, 0, 255, false, true);
			App->render->DrawQuad(SDL_Rect{ -App->render->camera.x + 4, -App->render->camera.y + 4, App->render->camera.w - 8, App->render->camera.h - 8 }, 255, 0, 0, 255, false, true);
		}

		if (App->game_scene->pause_timer.ReadSec() > 1.6)
			App->game_scene->active_timer = false;
	}
	return ret;
}

// Called before quitting
bool j1UIManager::CleanUp()
{
	LOG("Freeing UIManager");
	bool ret = true;

	CleanUpGameUI();

	list<UIEntity*>::iterator i = gui_elements.begin();

	while (i != gui_elements.end() && ret == true)
	{
			ret = (*i)->CleanUp();

			delete (*i);
			(*i) = NULL;
		++i;
	}

	gui_elements.clear();

	


	delete cursor;


	return ret;
}

void j1UIManager::CleanUpGameUI()
{

	animated_sprites.clear();
	map<string, UIEntity*>::iterator it = gui_database.begin();
	while (it != gui_database.end())
	{
		it->second->CleanUp();
		delete it->second;
		it->second = NULL;
		++it;
	}

	gui_database.clear();
}

void j1UIManager::StartGameUI()
{
	LoadUiInfo(); //Change names. This is really stupid
}

void j1UIManager::CleanUpList()
{
	list<UIEntity*>::iterator i = gui_elements.begin();

	while (i != gui_elements.end())
	{
		(*i)->CleanUp();

		delete (*i);
		(*i) = NULL;
		++i;
	}

	gui_elements.clear();

}


void j1UIManager::EraseElement(UIEntity* entity)
{
	gui_elements.remove(entity);
	delete entity;
}

// const getter for atlas
SDL_Texture* j1UIManager::GetAtlas()
{
	return atlas;
}

// class UIManager ---------------------------------------------------

UILabel* j1UIManager::CreateLabel(const char* text, const int x, const int y, bool on_list, j1Module* listener)
{
	UILabel* label = new UILabel(text, x, y);
	label->listener = listener;
	if (on_list == true)
		gui_elements.push_back(label);

	return label;
}

UIImage* j1UIManager::CreateImage(SDL_Rect _section, const int x, const int y, bool initial_visible, bool on_list, j1Module* listener)
{
	UIImage* img = new UIImage(_section, x, y);
	img->listener = listener;
	img->is_visible = initial_visible;
	if (on_list == true)
		gui_elements.push_back(img);

	return img;
}

UIButton* j1UIManager::CreateButton(const char* _text, const int x, const int y, SDL_Rect section_idle, SDL_Rect section_pressed, SDL_Rect section_hover, j1Module* listener)
{
	UIButton* button = new UIButton(_text, x, y, section_idle, section_pressed, section_hover);
	button->listener = listener;
	gui_elements.push_back(button);

	return button;
}

UICursor* j1UIManager::CreateCursor(vector<SDL_Rect> sections, float anim_speed, j1Module* listener)
{
	UICursor* cursor = new UICursor(sections, anim_speed);
	cursor->listener = listener;

	return cursor;
}

UIMiniMap* j1UIManager::CreateMiniMap(SDL_Rect _rec, SDL_Rect section_drawn, iPoint original_map_size, j1Module* listener)
{
	UIMiniMap* mini_map = new UIMiniMap(_rec, section_drawn, original_map_size);
	mini_map->listener = listener;
	gui_elements.push_back(mini_map);

	return mini_map;
}

/*UIInputBox* j1UIManager::CreateInputBox(const char* text, const int x, const int y, const char* path, j1Module* listener)
{
UIInputBox* ibox = new UIInputBox(text, x, y, path);
ibox->listener = listener;
gui_elements.push_back(ibox);

return ibox;
}*/

UIEntity* j1UIManager::GetMouseHover() const
{
	p2Point<int> mouse;
	App->input->GetMouseWorld(mouse.x, mouse.y);

	list<UIEntity*>::const_reverse_iterator item = gui_elements.rbegin();

	while (item != gui_elements.rend())
	{
		SDL_Rect rect = (*item)->GetScreenRect();
		if (mouse.PointInRect(rect.x, rect.y, rect.w, rect.h) == true && (*item)->IsVisible() == true)
		{
			return (*item);
		}
		++item;
	}

	return NULL;
}


//Utilities -------------------------------------------------------------------------------------------------------------------------------------
void j1UIManager::GetMouseInput()
{
	if (App->input->GetMouseButtonDown(1) == KEY_DOWN)
	{
		gui_pressed = GetMouseHover();
		/*if (gui_pressed != NULL)
		if (gui_pressed->focusable)
		{
			if (focus)
				focus->isFocus = false;
			focus = gui_pressed;
			gui_pressed->isFocus = true;
		}*/
	}

	if (gui_pressed)
		gui_pressed->Drag();

	if (App->input->GetMouseButtonDown(1) == KEY_UP)
	{
		gui_pressed = NULL;
	}
}

void j1UIManager::SetNextFocus()
{
	list<UIEntity*>::iterator item = gui_elements.begin();

	unsigned int min_x, min_y;
	App->win->GetWindowSize(min_x, min_y);
	int win_x = min_x;

	int focus_x, focus_y;
	if (focus != NULL)
		focus->GetScreenPos(focus_x, focus_y);
	else
		focus_x = focus_y = 0;

	UIEntity* best_match = NULL;
	while (item != gui_elements.end())
	{
		if ((*item)->focusable && (*item) != focus && (*item)->IsVisible())
		{
			SDL_Rect pos = (*item)->GetScreenRect();
			if (pos.y < min_y && pos.y > focus_y)
			{
				best_match = (*item);
				min_y = pos.y;
				min_x = win_x;
			}
			else
			{
				if (pos.x < min_x && pos.y == focus_y && focus_x < pos.x)
				{
					best_match = (*item);
					min_x = pos.x;
				}
			}
		}
		++item;
	}

	if (best_match != NULL)
	{
		if (focus != NULL)
			focus->isFocus = false;
		focus = best_match;
		focus->isFocus = true;
	}
}

void j1UIManager::RectangleSelection()
{
	if (App->scene_manager->in_game == true)
	{
		if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN && pressed_last_frame == false)
		{
			if (GetMouseHover() == NULL)
			{
				App->input->GetMouseWorld(selection_rect.x, selection_rect.y);
				pressed_last_frame = true;
				selection_valid = true;
			}
			else
				selection_valid = false;
		}

		if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT)
		{
			iPoint position;
			App->input->GetMouseWorld(position.x, position.y);
			selection_rect.w = position.x - selection_rect.x;
			selection_rect.h = position.y - selection_rect.y;
		}

		if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP && pressed_last_frame == true)
		{
			pressed_last_frame = false;
		}

		if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT && selection_valid == true)
			App->render->DrawQuad(selection_rect, 0, 255, 0, 255, false);
	}
}

void j1UIManager::OcultWireframes()
{
	if (App->scene_manager->in_game == false)
		return;
	App->game_scene->marine_armour_icon->is_visible = false;
	App->game_scene->marine_weapon_icon->is_visible = false;
	App->game_scene->marine_wireframe->is_visible = false;

	App->game_scene->ghost_weapon_icon->is_visible = false;
	App->game_scene->ghost_wireframe->is_visible = false;


	App->game_scene->firebat_weapon_icon->is_visible = false;
	App->game_scene->firebat_wireframe->is_visible = false;

	App->game_scene->observer_wireframe->is_visible = false;
	App->game_scene->medic_wireframe->is_visible = false;
}

void j1UIManager::ShowMiniWireframes()
{
	uint i = 0;
	for (list<Unit*>::iterator it = App->entity->selected_units.begin(); it != App->entity->selected_units.end(); it++, i++)
	{
		switch ((*it)->GetType())
		{
		case(MARINE) :
			ShowMiniWireframe("MARINE", i);
			break;

		case(FIREBAT) :
			ShowMiniWireframe("FIREBAT", i);
			break;

		case(GHOST) :
			ShowMiniWireframe("GHOST", i);
			break;

		case(MEDIC) :
			ShowMiniWireframe("MEDIC", i);
			break;

		case(OBSERVER) :
			ShowMiniWireframe("OBSERVER", i);
			break;
		}
	}
}

void j1UIManager::ShowMiniWireframe(const char* mw_key, uint pos)
{
	UIImage* mini_wireframe = (UIImage*)gui_database.find(mw_key)->second;
	int x, y;
	mini_wireframe->GetLocalPos(x, y);

	if (pos < 6)
		mini_wireframe->SetLocalPos(x + pos * mw_width, y);
	
	else if (pos < 12)
		mini_wireframe->SetLocalPos(x + (pos - 6) * mw_width, y + mw_height);

	mini_wireframe->Draw();
	mini_wireframe->SetLocalPos(x, y);
}

void j1UIManager::ShowIndividualWireframe()
{
	//------------Show life in HUD------------------------

	/*
	life_HUD->Print("");
	int total_unit_life = (*it)->hp_bar->GetMaxSize();
	int life_from_total = (*it)->hp_bar->current_number;

	char ui_life[20];

	sprintf_s(ui_life, sizeof(ui_life), "%d / %d", life_from_total, total_unit_life);
	life_HUD->Print(ui_life);*/

	//------------Show life in HUD------------------------
	
	list<Unit*>::iterator it = App->entity->selected_units.begin();

	switch ((*it)->GetType())
	{
	case MARINE:
		App->game_scene->marine_armour_icon->is_visible = true;
		App->game_scene->marine_weapon_icon->is_visible = true;
		App->game_scene->marine_wireframe->is_visible = true;
		break;

	case GHOST:
		App->game_scene->marine_armour_icon->is_visible = true;
		App->game_scene->ghost_weapon_icon->is_visible = true;
		App->game_scene->ghost_wireframe->is_visible = true;
		break;

	case FIREBAT:
		App->game_scene->marine_armour_icon->is_visible = true;
		App->game_scene->firebat_weapon_icon->is_visible = true;
		App->game_scene->firebat_wireframe->is_visible = true;
		break;

	case MEDIC:
		App->game_scene->medic_wireframe->is_visible = true;
		break;

	case OBSERVER:
		App->game_scene->observer_wireframe->is_visible = true;
		break;
	}
}

void j1UIManager::DrawLifeMana()
{
	if (App->scene_manager->in_game == true)
	{
		UIProgressBar* life = (UIProgressBar*)(*gui_database.find("HEALTH_BAR")).second;
		UIProgressBar* mana = (UIProgressBar*)(*gui_database.find("MANA_BAR")).second;

		list<Unit*>::iterator unit = App->entity->friendly_units.begin();

		while (unit != App->entity->friendly_units.end())
		{
			if ((*unit)->state != UNIT_DIE)
			{
				life->max_number = (*unit)->GetMaxLife();
				mana->max_number = (*unit)->GetMaxMana();

				life->SetValue((*unit)->GetLife());
				mana->SetValue((*unit)->GetMana());

				life->Draw((*unit)->GetPosition().x + 4, (*unit)->GetPosition().y - 7);
				if ((*unit)->type == GHOST)
				mana->Draw((*unit)->GetPosition().x + 4, (*unit)->GetPosition().y - 7);
			}
			
			++unit;
		}
	}
	
}


// ANIMATION
//------------------------------------------------------------------------------------------------------------
void j1UIManager::UpdateAnimation(float dt)
{
	list<AnimatedSprite>::iterator a_sprite = animated_sprites.begin();
	while (a_sprite != animated_sprites.end())
	{

		//Update delay time (if has)
		if ((*a_sprite).timer < (*a_sprite).delay)
		{
			(*a_sprite).timer += dt;
			++a_sprite;
			continue;
		}


		//Fade animation
		if ((*a_sprite).alpha_step != 0)
		{

			float step = (float)(255 * dt) / (*a_sprite).alpha_step;
			(*a_sprite).sprite->alpha += roundf(step);

			//Max alpha
			if ((*a_sprite).sprite->alpha >= 255)
			{
				(*a_sprite).sprite->alpha = 255;
				a_sprite = animated_sprites.erase(a_sprite);
				continue;
			}

			//Min alpha
			if ((*a_sprite).sprite->alpha <= 0)
			{
				(*a_sprite).sprite->alpha = 0;
				a_sprite = animated_sprites.erase(a_sprite);
				continue;
			}
		}
		
		
		++a_sprite;
		
		
	}
}

void j1UIManager::AnimFade(UIEntity* ui_sprite, uint duration, bool fade_in, uint delay)
{
	/*if (ui_sprite != NULL)
	{
		AnimatedSprite a_sprite;
		a_sprite.sprite = ui_sprite->GetSprite();
		a_sprite.alpha_step = (fade_in) ? duration : -(int)duration;
		a_sprite.sprite->alpha = (fade_in) ? 0 : 255;
		a_sprite.delay = delay;
		a_sprite.timer = 0;

		animated_sprites.push_back(a_sprite);
	}*/
}

void j1UIManager::AnimResize(UIEntity* ui_sprite, uint duration, bool size_big, uint delay)
{
	if (ui_sprite != NULL)
	{
		AnimatedSprite a_sprite;
		a_sprite.sprite = ui_sprite->GetSprite();
		a_sprite.size_step = (size_big) ? duration : -(int)duration;
		//Set size
		//Normal-->small
			//start size = normal
			//final size = small (10% normal)

		//small-->normal
			//start size = small(10%normal)
			//big size = 105% normal
			//final size = normal = 1
		a_sprite.delay = delay;
		a_sprite.timer = 0;

		animated_sprites.push_back(a_sprite);
	}
}
