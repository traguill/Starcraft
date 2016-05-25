#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "MenuScene.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "j1UIManager.h"
#include "SceneManager.h"
#include "j1Audio.h"
#include "j1Input.h"
#include "UIImage.h"
#include "UIButton.h"

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
	LOG("Starting MenuScene");

	//background = App->ui->CreateImage({ 663, 590, 735, 494 }, 0, 0, true);
	logo = App->ui->CreateImage({ 0, 199, 569, 150 }, 0, 50, true);

	
	start = App->ui->CreateButton(" START", 100, 250, { 348, 109, 125, 26 }, { 348, 161, 125, 26 }, { 348, 135, 125, 26 }, this);
	quit = App->ui->CreateButton(" QUIT", 100, 305, { 348, 109, 125, 26 }, { 348, 161, 125, 26 }, { 348, 135, 125, 26 }, this);
	normal = App->ui->CreateButton("NORMAL", 100, 250, { 348, 109, 125, 26 }, { 348, 161, 125, 26 }, { 348, 135, 125, 26 }, this);
	hardcore = App->ui->CreateButton(" HARD", 250, 250, { 348, 109, 125, 26 }, { 348, 161, 125, 26 }, { 348, 135, 125, 26 }, this);
	pro = App->ui->CreateButton("  PRO", 400, 250, { 348, 109, 125, 26 }, { 348, 161, 125, 26 }, { 348, 135, 125, 26 }, this);

	normal->is_visible = false;
	hardcore->is_visible = false;
	pro->is_visible = false;

	//Animation
	App->ui->AnimFade(logo, 2, true, 2);
	App->ui->AnimResize(start, 0.5f, true, 4);
	App->ui->AnimResize(quit, 0.5f, true, 4.2f);
	
	
	close_game = false;

	App->audio->PlayMusic("MenuTheme.wav");

	background_anim.frames.clear();
	SDL_Rect section;
	section.x = 1477; section.y = 0; section.w = 640; section.h = 480;
	background_anim.frames.push_back(section);
	section.x = 1477 + 640; section.y = 0; section.w = 640; section.h = 480;
	background_anim.frames.push_back(section);
	section.x = 1477 + 640; section.y = 480; section.w = 640; section.h = 480;
	background_anim.frames.push_back(section);
	section.x = 1477 + 640 + 640; section.y = 480; section.w = 640; section.h = 480;
	background_anim.frames.push_back(section);

	section.x = section.y = section.w = section.h = 0;

	background_anim.loop = true;
	background_anim.speed = 0.2f;

	App->ui->cursor_state = STANDARD;

	App->render->camera.x =  App->render->camera.y = 0;

	

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
	
	if (App->input->GetKey(SDL_SCANCODE_KP_5) == KEY_UP)
	{
		App->scene_manager->WantToChangeScene(DEV);
	}

	App->render->Blit(App->ui->GetAtlas(), 0, 0, &background_anim.getCurrentFrame());

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

	start = NULL;
	quit = NULL;
	background = NULL;
	logo = NULL;
	background_anim.frames.clear();

	return true;
}

void MenuScene::OnGUI(UIEntity* gui, GUI_EVENTS event)
{
	if (gui->type == BUTTON)
	{
		if ((UIButton*)gui == start && event == MOUSE_BUTTON_RIGHT_UP)
		{			
			App->ui->AnimResize(start, 0.1f, false, 0.3f);

			App->ui->AnimResize(normal, 0.5f, true, 1.0f);
			App->ui->AnimResize(hardcore, 0.5f, true, 1.0f);
			App->ui->AnimResize(pro, 0.5f, true, 1.0f);

		}

		else if ((UIButton*)gui == quit && event == MOUSE_BUTTON_RIGHT_UP)
		{
			close_game = true;
		}

		else if ((UIButton*)gui == normal && event == MOUSE_BUTTON_RIGHT_UP)
		{
			App->scene_manager->WantToChangeScene(GAME);
			App->scene_manager->dificulty = false;
			App->scene_manager->pro = false;
		}

		else if ((UIButton*)gui == hardcore && event == MOUSE_BUTTON_RIGHT_UP)
		{
			App->scene_manager->WantToChangeScene(GAME);
			App->scene_manager->dificulty = true;
			App->scene_manager->pro = false;
		}

		else if ((UIButton*)gui == pro && event == MOUSE_BUTTON_RIGHT_UP)
		{
			App->scene_manager->WantToChangeScene(GAME);
			App->scene_manager->pro = true;
		}
	}
}