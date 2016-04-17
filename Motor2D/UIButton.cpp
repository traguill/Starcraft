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
	interactable = true;
	state = IDLE;
	text.Print(_text);

	rect.x = x;
	rect.y = y;
	rect.w = section_idle.w;
	rect.h = section_idle.h;

	idle = section_idle;
	pressed = section_pressed;
	hover = section_hover;

	text.SetParent(this);
}


// Destructor
UIButton::~UIButton()
{}

// Called before render is available
bool UIButton::Update(float dt)
{
	bool ret = true;

	GetState();

	int x, y;
	GetScreenPos(x, y);

	if (state == IDLE)
		App->render->Blit(App->ui->GetAtlas(), x, y, &idle);
	if (state == PRESSED)
		App->render->Blit(App->ui->GetAtlas(), x, y, &pressed);
	if (state == HOVER)
		App->render->Blit(App->ui->GetAtlas(), x, y, &hover);



	text.Update(dt);


	return ret;
}

bool UIButton::CleanUp()
{
	bool ret = true;

	text.CleanUp();

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