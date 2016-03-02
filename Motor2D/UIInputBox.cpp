#include "UIInputBox.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Fonts.h"
#include "j1Textures.h"
#include "UILabel.h"
#include "j1Input.h"

UIInputBox::UIInputBox() : UIEntity()
{}

UIInputBox::UIInputBox(const char* txt, const int x, const int y, const char* path) : UIEntity()
{

	type = INPUT_BOX;
	interactable = true;
	
	text = new UILabel(txt, 0, 0);
	background = new UIImage(path, 0 - DELAY_INPUT, 0 - DELAY_INPUT);

	text->SetParent(this);
	background->SetParent(this);

	rect.x = x;
	rect.y = y;
	rect.w = text->GetScreenRect().w;
	rect.h = text->GetScreenRect().h;
}



// Destructor
UIInputBox::~UIInputBox()
{}

// Called before render is available
bool UIInputBox::Update(float dt)
{
	bool ret = true;

	background->Update(dt);

	if (isFocus && !writting)
	{
		App->input->StartGetText(0);	//Start writting
		text->Print("");
		writting = true;
	}

	if (isFocus && writting)
	{
		text->Print(App->input->GetTextInput(), is_password);
		DrawCursor(App->input->GetCursorPosition());			//Keep writting
	}
	
	if (!isFocus && writting)
	{
		writting = false;				//Stop writting
	}
	text->Update(dt);

	return ret;
}

bool UIInputBox::CleanUp()
{
	bool ret = true;


	text->CleanUp();
	background->CleanUp();

	delete text;
	delete background;

	return ret;
}

void UIInputBox::DrawCursor(int position)
{
	SDL_Rect cursor = text->GetScreenRect();
	string t(text->GetText(is_password));
	t.erase(position, strlen(text->GetText().data())-position);
	App->font->CalcSize(t.data(), cursor.w, cursor.h);
	cursor.x += cursor.w;
	cursor.w = 1;
	App->render->DrawQuad(cursor, 255, 255, 255, 255);

}

