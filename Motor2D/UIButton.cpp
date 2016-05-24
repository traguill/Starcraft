#include "UIButton.h"
#include "j1App.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "UILabel.h"
#include "j1UIManager.h"

UIButton::UIButton() : UIEntity()
{
	type = BUTTON;
	state = IDLE;
	interactable = true;
}

UIButton::UIButton(const char* _text, const int x, const int y, SDL_Rect section_idle, SDL_Rect section_pressed, SDL_Rect section_hover) : UIEntity()
{
	type = BUTTON;
	interactable = false;
	state = IDLE;
	
	rect.x = x;
	rect.y = y;
	rect.w = section_idle.w;
	rect.h = section_idle.h;
	init_pos.x = x;
	init_pos.y = y;

	//X & Y position are magic numbers
	text = App->ui->CreateLabel(_text, rect.x + (rect.w / 3), rect.y + (rect.h / 6), false);
	text->SetParent(this);

	idle = section_idle;
	pressed = section_pressed;
	hover = section_hover;

	ui_sprite.position = init_pos;
	ui_sprite.rect = section_idle;
	ui_sprite.texture = App->ui->GetAtlas();
}


// Destructor
UIButton::~UIButton()
{
	ui_sprite.texture = NULL;
}

// Called before render is available
bool UIButton::Update(float dt)
{
	bool ret = true;

	GetState();

	iPoint cam(App->render->camera.x, App->render->camera.y);

	ui_sprite.position.x = init_pos.x - cam.x;
	ui_sprite.position.y = init_pos.y - cam.y;

	rect.x = ui_sprite.position.x;
	rect.y = ui_sprite.position.y;

	if (state == IDLE)
		ui_sprite.rect = idle;
	if (state == PRESSED)						
		ui_sprite.rect = pressed;
	if (state == HOVER)							
		ui_sprite.rect = hover;

	App->render->BlitUI(ui_sprite);


	//Update text properties
	Sprite* text_sprite = text->GetSprite();
	if (text_sprite != NULL)
	{
		text_sprite->alpha = ui_sprite.alpha;
		text_sprite->size = ui_sprite.size;
	}
	text->Update(dt);


	return ret;
}

bool UIButton::CleanUp()
{
	bool ret = true;

	text->CleanUp();
	delete text;
	text = NULL;

	return ret;
}

void UIButton::GetState()
{
	if (gui_event == MOUSE_ENTER)
		state = HOVER;
	if (gui_event == MOUSE_EXIT)
		state = IDLE;
	if (gui_event == MOUSE_BUTTON_RIGHT_DOWN)
		state = PRESSED;
}

Sprite* UIButton::GetSprite()
{
	return &ui_sprite;
}


void UIButton::GetScreenPos(int &x, int &y)const
{
	x = y = 0;

	x += rect.x;
	y += rect.y;
}

void UIButton::GetLocalPos(int &x, int &y)const
{
	x = rect.x;
	y = rect.y;
}

SDL_Rect UIButton::GetScreenRect()const
{
	SDL_Rect ret = rect;
	GetScreenPos(ret.x, ret.y);

	return ret;
}

SDL_Rect UIButton::GetLocalRect()const
{
	return rect;
}

void UIButton::SetLocalPos(int x, int y)
{
	ui_sprite.position.x = ui_sprite.rect.x = rect.x = x;
	ui_sprite.position.y = ui_sprite.rect.y = rect.y = y;
}