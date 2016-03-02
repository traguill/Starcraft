#include "UIButton.h"
#include "j1App.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "UILabel.h"

UIButton::UIButton() : UIEntity()
{
	type = BUTTON;
	state = IDLE;
	interactable = true;
}

UIButton::UIButton(const char* _text, const int x, const int y, const char* path_idle, const char* path_pressed, const char* path_hover) : UIEntity()
{
	type = BUTTON;
	interactable = true;
	state = IDLE;
	text.Print(_text);

	rect.x = x;
	rect.y = y;

	idle = App->tex->Load(path_idle);

	if (path_pressed)
		pressed = App->tex->Load(path_pressed);
	if (path_hover)
		hover = App->tex->Load(path_hover);
	text.SetParent(this);
	SDL_QueryTexture(idle, NULL, NULL, &rect.w, &rect.h);

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
		App->render->Blit(idle, x , y);
	if (state == PRESSED)
		App->render->Blit(pressed, x , y);
	if (state == HOVER)
		App->render->Blit(hover, x , y);

	/*SDL_Rect button_rec = GetScreenRect();
	SDL_Rect t = text.GetLocalRect();
	x = button_rec.x + (button_rec.w / 2 - t.w / 2);
	y = button_rec.y + (button_rec.h / 2 - t.h / 2);
	App->render->Blit(text.texture, x, y);*/

	text.Update(dt);


	return ret;
}

bool UIButton::CleanUp()
{
	bool ret = true;

	if (idle)
		App->tex->UnLoad(idle);
	if (pressed)
		App->tex->UnLoad(pressed);
	if (hover)
		App->tex->UnLoad(hover);

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
