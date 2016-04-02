#include "UIImage.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1UIManager.h"

UIImage::UIImage() : UIEntity()
{
	section = { 0, 0, 0, 0 };
	type = IMAGE;
	interactable = true;
}

UIImage::UIImage(SDL_Rect _section, const int x, const int y) : UIEntity()
{
	interactable = true;
	type = IMAGE;
	section = _section;

	rect.x = x;
	rect.y = y;
}

// Destructor
UIImage::~UIImage()
{}

// Called before render is available
bool UIImage::Update(float dt)
{
	bool ret = true;

	iPoint cam_pos(App->render->camera.x, App->render->camera.y);

	int x, y;
	GetScreenPos(x, y);
	App->render->Blit(App->ui->GetAtlas(), x - cam_pos.x, y - cam_pos.y, &section);

	return ret;
}

bool UIImage::CleanUp()
{
	bool ret = true;

	return ret;
}