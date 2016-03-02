#include "UIImage.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Textures.h"

UIImage::UIImage() : UIEntity()
{
	texture = NULL;
	type = IMAGE;
	interactable = true;
}

UIImage::UIImage(const char* path, const int x, const int y) : UIEntity()
{
	interactable = true;
	type = IMAGE;
	texture = App->tex->Load(path);
	
	rect.x = x;
	rect.y = y;

	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	
}

UIImage::UIImage(SDL_Texture* tex, const int x, const int y) : UIEntity()
{
	type = IMAGE;
	texture = tex;
	
	rect.x = x;
	rect.y = y;

	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	
}

// Destructor
UIImage::~UIImage()
{}

// Called before render is available
bool UIImage::Update(float dt)
{
	bool ret = true;

	if (texture != NULL)
	{
		iPoint cam_pos(App->render->camera.x, App->render->camera.y);

		int x, y;
		GetScreenPos(x, y);
		App->render->Blit(texture, x - cam_pos.x, y - cam_pos.y);
	}

	return ret;
}

bool UIImage::CleanUp()
{
	bool ret = true;


	if (texture != NULL)
	{
		App->tex->UnLoad(texture);
	}


	return ret;
}

void UIImage::LoadTexture(const char* path)
{
	if (texture != NULL)
	{
		App->tex->UnLoad(texture);
	}

	texture = App->tex->Load(path);
}