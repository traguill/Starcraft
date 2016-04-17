#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "MenuScene.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "j1UIManager.h"
#include "SceneManager.h"


MenuScene::MenuScene() : j1Module()
{
	name.append("menu_scene");
}

// Destructor
MenuScene::~MenuScene()
{}

// Called before render is available
bool MenuScene::Awake(pugi::xml_node& conf)
{
	LOG("Loading MenuScene");
	bool ret = true;


	return ret;
}

// Called before the first frame
bool MenuScene::Start()
{
	background = App->ui->CreateImage({ 663, 590, 735, 494 }, 0, 0, true);
	logo = App->ui->CreateImage({ 0, 202, 569, 132 }, 0, 50, true);

	const char* s = " START";
	const char* q = " QUIT";
	
	start = App->ui->CreateButton(s, 100, 250, { 348, 109, 125, 26 }, { 348, 161, 125, 26 }, { 348, 135, 125, 26 }, this);
	quit = App->ui->CreateButton(q, 100, 305, { 348, 109, 125, 26 }, { 348, 161, 125, 26 }, { 348, 135, 125, 26 }, this);

	close_game = false;

	return true;
}

// Called each loop iteration
bool MenuScene::PreUpdate()
{

	return true;
}

// Called each loop iteration
bool MenuScene::Update(float dt)
{
	


	return true;
}

// Called each loop iteration
bool MenuScene::PostUpdate()
{
	bool ret = true;

	if (close_game == true)
		return false;

	return ret;
}

// Called before quitting
bool MenuScene::CleanUp()
{
	LOG("Freeing Menu Scene");



	return true;
}

void MenuScene::OnGUI(UIEntity* gui, GUI_EVENTS event)
{
	if (gui->type == BUTTON)
	{
		if ((UIButton*)gui == start && event == MOUSE_BUTTON_RIGHT_UP)
		{
			App->scene_manager->WantToChangeScene();
		}

		else if ((UIButton*)gui == quit && event == MOUSE_BUTTON_RIGHT_UP)
		{
			close_game = true;
		}
	}
}