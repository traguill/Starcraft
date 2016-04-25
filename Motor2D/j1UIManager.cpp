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

	LoadUiInfo();

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
	
	RELEASE(buf);

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
	for (element = ui_elements.child("gui"); element && ret; element = ui_elements.next_sibling("gui"))
	{
		//Progressbar
		if (element.attribute("type").as_string() == "PROGRESS_BAR")
		{
			UIProgressBar* bar = new UIProgressBar();

			//Fill data here
			bar->bar_tex = App->tex->Load(element.child("texture").attribute("value").as_string());

			bar->SetLocalPos(element.child("local_pos").attribute("x").as_int(), element.child("local_pos").attribute("y").as_int());

			int width = element.child("size").attribute("width").as_int();
			int height = element.child("size").attribute("height").as_int();

			bar->full_bar_section = { element.child("full").attribute("x").as_int(), element.child("full").attribute("y").as_int(), width, height };
			bar->middle_bar_section = { element.child("middle").attribute("x").as_int(), element.child("full").attribute("y").as_int(), width, height };
			bar->low_bar_section = { element.child("low").attribute("x").as_int(), element.child("full").attribute("y").as_int(), width, height };
			bar->empty_bar_section = { element.child("empty").attribute("x").as_int(), element.child("full").attribute("y").as_int(), width, height };

			string bar_type = element.child("bar_type").attribute("value").as_string();

			//Types of bars
			if (bar_type == "HEALTH")
				bar->bar_type = HEALTH;

			if (bar_type == "MANA")
				bar->bar_type = MANA;

			gui_database.insert(pair<string, UIEntity*>(element.attribute("key").as_string(), bar));
		}

		
	}

	return ret;
}
// Update all UIManagers
bool j1UIManager::PreUpdate()
{
	return true;
}

// Called after all Updates
bool j1UIManager::PostUpdate()
{
	return true;
}

bool j1UIManager::Update(float dt)
{
	bool ret = true;

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_UP)
		debug = !debug;

	GetMouseInput();

	/*if (App->input->GetKey(SDL_SCANCODE_TAB) == KEY_DOWN)
	{
		SetNextFocus();
	}*/

	RectangleSelection();

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

	//TODO: Draw lifes & mana
	
	if (App->entity->selected_units.size() > 1)
		ShowMiniWireframes(dt);
	
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
			if ((*it)->GetType() == GHOST && (*it)->GetSnipping() == true){
				rec = (*it)->current_animation->getCurrentFrame();
				pos = (*it)->GetDrawPosition();
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


	return ret;
}

// Called before quitting
bool j1UIManager::CleanUp()
{
	LOG("Freeing UIManager");
	bool ret = true;



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
SDL_Texture* j1UIManager::GetAtlas() const
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
	App->input->GetMousePosition(mouse.x, mouse.y);

	list<UIEntity*>::const_reverse_iterator item = gui_elements.rbegin();

	while (item != gui_elements.rend())
	{
		SDL_Rect rect = (*item)->GetScreenRect();
		if (mouse.PointInRect(rect.x, rect.y, rect.w, rect.h) == true)
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
		if (gui_pressed != NULL)
		if (gui_pressed->focusable)
		{
			if (focus)
				focus->isFocus = false;
			focus = gui_pressed;
			gui_pressed->isFocus = true;
		}
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
			App->input->GetMouseWorld(selection_rect.x, selection_rect.y);
			pressed_last_frame = true;
		}

		if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT)
		{
			iPoint position;
			App->input->GetMouseWorld(position.x, position.y);
			selection_rect.w = position.x - selection_rect.x;
			selection_rect.h = position.y - selection_rect.y;
		}

		if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP && pressed_last_frame == true)
			pressed_last_frame = false;

		if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT)
			App->render->DrawQuad(selection_rect, 0, 255, 0, 255, false);
	}
}

void j1UIManager::OcultWireframes()
{
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

void j1UIManager::ShowMiniWireframes(float dt)
{
	list<UIImage*>::iterator it = mini_wireframes.begin();
	while (it != mini_wireframes.end())
	{
		(*it)->Update(dt);
		if (debug)
			(*it)->Debug();
		++it;
	}
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

	case ENGINEER:
			
		break;
	}
}

void j1UIManager::CreateMiniWireframe(UNIT_TYPE type, uint pos)
{
	uint width = 35; uint height = 40;

	switch (type)
	{
	case MARINE:
		if (pos < 6)
			mini_wireframes.push_back(CreateImage({ 736, 445, 33, 34 }, 170 + pos * width, 400, true, false));

		else if (pos < 12)
			mini_wireframes.push_back(CreateImage({ 736, 445, 33, 34 }, 170 + (pos - 6) * width, 400 + height, true, false));

		break;

	case GHOST:
		if (pos < 6)
			mini_wireframes.push_back(CreateImage({ 814, 445, 33, 34 }, 170 + pos * width, 400, true, false));

		else if (pos < 12)
			mini_wireframes.push_back(CreateImage({ 814, 445, 33, 34 }, 170 + (pos - 6) * width, 400 + height, true, false));

		break;

	case FIREBAT:
		if (pos < 6)
			mini_wireframes.push_back(CreateImage({ 776, 445, 33, 34 }, 170 + pos * width, 400, true, false));

		else if (pos < 12)
			mini_wireframes.push_back(CreateImage({ 776, 445, 33, 34 }, 170 + (pos - 6) * width, 400 + height, true, false));
		break;

	case MEDIC:
		if (pos < 6)
			mini_wireframes.push_back(CreateImage({ 736, 485, 33, 34 }, 170 + pos * width, 400, true, false));

		else if (pos < 12)
			mini_wireframes.push_back(CreateImage({ 736, 485, 33, 34 }, 170 + (pos - 6) * width, 400 + height, true, false));
		break;

	case OBSERVER :
		if (pos < 6)
			mini_wireframes.push_back(CreateImage({ 776, 485, 33, 34 }, 170 + pos * width, 400, true, false));

		else if (pos < 12)
			mini_wireframes.push_back(CreateImage({ 776, 485, 33, 34 }, 170 + (pos - 6) * width, 400 + height, true, false));
		break;
	}
}



void j1UIManager::DeleteMiniWIreframe(uint pos)
{
	if (mini_wireframes.size() > 2)
	{
		list<UIImage*>::iterator it = mini_wireframes.begin();
		for (uint i = 0; i < pos; i++)
			it++;

		delete (*it);
		it = mini_wireframes.erase(it);


		for (uint i = pos; i < mini_wireframes.size(); i++, it++)
		{
			int x, y;
			(*it)->GetLocalPos(x, y);

			if (i != 5)
				(*it)->SetLocalPos(x - 35, y);

			else
				(*it)->SetLocalPos(x + 35 * 5, y - 40);
		}
	}

	else
	{
		list<UIImage*>::iterator it = mini_wireframes.begin();
		while (it != mini_wireframes.end())
		{
			RELEASE(*it);
			it++;
		}
		mini_wireframes.clear();
	}
}
