#include "UILabel.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Fonts.h"
#include "j1Textures.h"

UILabel::UILabel() : UIEntity()
{
	type = LABEL;
	texture = NULL;
	text = "";
}

UILabel::UILabel(const char* txt, const int x, const int y) : UIEntity()
{
	type = LABEL;
	text = txt;
	
	texture = App->font->Print(text.data());

	rect.x = x;
	rect.y = y;
	
	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
}



// Destructor
UILabel::~UILabel()
{}

// Called before render is available
bool UILabel::Update(float dt)
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

bool UILabel::CleanUp()
{
	bool ret = true;

	if (texture != NULL)
	{
		App->tex->UnLoad(texture);
	}
		

	return ret;
}


void UILabel::Print(string _text, bool isPassword)
{
	App->tex->UnLoad(texture);

	text = _text;
	if (text != "")
	{
		if (isPassword == false)
		{
			texture = App->font->Print(text.data());
		}
		else
		{
			password = text;
			password.replace(password.begin(), password.end() - strlen(text.data()), strlen(text.data()), '*');
			texture = App->font->Print(password.data());
		}
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	}
	else
		texture = NULL;
}

string UILabel::GetText(bool is_password) const
{
	if (is_password)
		return password;
	else
		return text;
}