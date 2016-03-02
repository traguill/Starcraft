#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1Map.h"
#include "j1PathFinding.h"
#include "j1Gui.h"
#include "j1WowMenu.h"
#include "j1UIManager.h"
#include "UILabel.h"
#include "UIImage.h"
#include "UIButton.h"
#include "UIInputBox.h"

j1WowMenu::j1WowMenu() : j1Module()
{
	name.append("scene");
}

// Destructor
j1WowMenu::~j1WowMenu()
{}

// Called before render is available
bool j1WowMenu::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

	return ret;
}

// Called before the first frame
bool j1WowMenu::Start()
{
	
	background = App->ui->CreateImage("wow/background.jpg", 0, 0);
	background->interactable = false;
	window = App->ui->CreateImage("wow/window.png", 500, 600);
	login = App->ui->CreateButton("Login", 20, 10, "wow/button.png", "wow/button_pressed.png", "wow/button_hover.png");
	login->listener = this;
	login->SetParent(window);
	App->ui->CreateButton("Manage Account", 20, 800, "wow/button.png", "wow/button_pressed.png", "wow/button_hover.png");
	App->ui->CreateButton("Community Site", 20, 825, "wow/button.png", "wow/button_pressed.png", "wow/button_hover.png");
	App->ui->CreateButton("Cinematics", 1110, 775, "wow/button.png", "wow/button_pressed.png", "wow/button_hover.png");
	App->ui->CreateButton("Credits", 1110, 800, "wow/button.png", "wow/button_pressed.png", "wow/button_hover.png");
	App->ui->CreateButton("Terms of Use", 1110, 825, "wow/button.png", "wow/button_pressed.png", "wow/button_hover.png");

	App->ui->CreateInputBox("Your name", 2, 500, "wow/input_box.png");
	password_box = App->ui->CreateInputBox("Your password", 2, 600, "wow/input_box.png");
	password_box->is_password = true;
	return true;
}

// Called each loop iteration
bool j1WowMenu::PreUpdate()
{

	return true;
}

// Called each loop iteration
bool j1WowMenu::Update(float dt)
{


	return true;
}

// Called each loop iteration
bool j1WowMenu::PostUpdate()
{
	bool ret = true;

	if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

// Called before quitting
bool j1WowMenu::CleanUp()
{
	LOG("Freeing scene");

	return true;
}

void j1WowMenu::OnGUI(UIEntity* gui, GUI_EVENTS event)
{

	if (gui->type == BUTTON)
	{
		if (gui == login)
			if (event == MOUSE_BUTTON_RIGHT_DOWN)
			{
				login->SetVisible(false);
			}
				
			
	}
}
