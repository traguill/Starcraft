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

	rect.x = init_pos.x - cam.x;
	rect.y = init_pos.y - cam.y;

	if (state == IDLE)
		App->render->Blit(App->ui->GetAtlas(), rect.x, rect.y, &idle);
	if (state == PRESSED)						
		App->render->Blit(App->ui->GetAtlas(), rect.x, rect.y, &pressed);
	if (state == HOVER)							
		App->render->Blit(App->ui->GetAtlas(), rect.x, rect.y, &hover);



	//Update text properties
	Sprite* text_sprite = text->GetSprite();
	if (text_sprite != NULL)
	{
		text_sprite->alpha = ui_sprite.alpha;
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