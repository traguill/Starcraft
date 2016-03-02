#include "UIEntity.h"
#include "j1App.h"
#include "j1Input.h"
#include "p2Log.h"
#include "j1Module.h"
#include "j1Render.h"

UIEntity::UIEntity()
{}

// Destructor
UIEntity::~UIEntity()
{}

// Called before render is available
bool UIEntity::Update(float dt)
{
	return true;
}

bool UIEntity::CleanUp()
{
	return true;
}

void UIEntity::GUIEvents()
{
	SDL_Point mouse_position;
	App->input->GetMousePosition(mouse_position.x, mouse_position.y);

	SDL_Rect rec_screen = GetScreenRect();

	if (gui_event != MOUSE_ENTER && gui_event != MOUSE_BUTTON_RIGHT_DOWN)
	{
		if (mouse_position.x > rec_screen.x && mouse_position.y > rec_screen.y && mouse_position.x < rec_screen.x + rec_screen.w &&  mouse_position.y < rec_screen.y + rec_screen.h)
		{
			
			gui_event = MOUSE_ENTER;
			if (listener != NULL)
				listener->OnGUI(this, gui_event);
		}
	}
	else
	{
		if (gui_event == MOUSE_ENTER || gui_event == MOUSE_BUTTON_RIGHT_DOWN)
		{
			if (mouse_position.x > rec_screen.x && mouse_position.y > rec_screen.y && mouse_position.x < rec_screen.x + rec_screen.w &&  mouse_position.y < rec_screen.y + rec_screen.h)
			{
				if (App->input->GetMouseButtonDown(1) == KEY_DOWN)
				{
					gui_event = MOUSE_BUTTON_RIGHT_DOWN;
					if (listener != NULL)
						listener->OnGUI(this, gui_event);
				}
				else
				{
					if (App->input->GetMouseButtonDown(1) == KEY_UP)
					{
						gui_event = MOUSE_BUTTON_RIGHT_UP;
						if (listener != NULL)
							listener->OnGUI(this, gui_event);
					}
				}
			}	
			else
			{
				gui_event = MOUSE_EXIT;
				if (listener != NULL)
					listener->OnGUI(this, gui_event);
			}	
		}
	}
	
}

void UIEntity::Debug()
{
	SDL_Rect screen_rect = GetScreenRect();

	if (isFocus == false)
		App->render->DrawQuad(screen_rect, 0, 0, 255, 255, false);
	else
		App->render->DrawQuad(screen_rect, 255, 255, 0, 255, false);
	
}


void UIEntity::GetScreenPos(int &x, int &y)const
{
	x = y = 0;
	if (parent != NULL)
		parent->GetScreenPos(x, y);
	x += rect.x;
	y += rect.y;
}

void UIEntity::GetLocalPos(int &x, int &y)const
{
	x = rect.x;
	y = rect.y;
}

SDL_Rect UIEntity::GetScreenRect()const
{
	SDL_Rect ret = rect;
	GetScreenPos(ret.x, ret.y);

	return ret;
}

SDL_Rect UIEntity::GetLocalRect()const
{
	return rect;
}

void UIEntity::SetLocalPos(int x, int y)
{
	rect.x = x;
	rect.y = y;
}

void UIEntity::Drag()
{
	if (interactable)
	{
		int motion_x, motion_y;
		App->input->GetMouseMotion(motion_x, motion_y);
		rect.x += motion_x;
		rect.y += motion_y;
	}
}

void UIEntity::SetParent(UIEntity* _parent)
{
	parent = _parent;
	parent->childs.push_back(this);
}

void UIEntity::SetVisible(bool visible)
{
	if (visible != isVisible)
	{
		isVisible = visible;

		list<UIEntity*>::iterator i = childs.begin();

		while (i != childs.end())
		{
			(*i)->SetVisible(visible);
			++i;
		}

		delete (*i);
	}
}

bool UIEntity::IsVisible()const
{
	return isVisible;
}