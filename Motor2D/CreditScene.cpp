#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "CreditScene.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "j1UIManager.h"
#include "SceneManager.h"
#include "j1Audio.h"
#include "j1Input.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UIButton.h"

CreditScene::CreditScene() : j1Module()
{
	name.append("credit_scene");
}

// Destructor
CreditScene::~CreditScene()
{}

// Called before render is available
bool CreditScene::Awake(pugi::xml_node& conf)
{
	LOG("Loading CreditScene");
	bool ret = true;

	return ret;
}

// Called before the first frame
bool CreditScene::Start()
{
	LOG("Starting CreditScene");

	close_game = false;

	project = App->ui->CreateImage({1740, 536, 360, 70}, 150, 200, false);
	sheep_logo = App->ui->CreateImage({ 0, 0, 0, 0 }, 100, 300, false);
	upc_logo = App->ui->CreateImage({ 2883, 0, 356, 67 }, 150, 200, false);
	license_blizzard = App->ui->CreateImage({ 2795, 174, 528, 66 }, 60, 300, false);
	license_image = App->ui->CreateImage({ 2795, 244, 526, 77 }, 60, 300, false);

	credits_list.push_back(project);
	credits_list.push_back(upc_logo);
	//credits_list.push_back(sheep_logo);
	credits_list.push_back(license_blizzard);
	credits_list.push_back(license_image);


	credits = App->ui->CreateImage({ 1806, 640, 304, 322 }, 150, 100, false);

	App->audio->PlayMusic("MenuTheme.wav");

	App->ui->cursor_state = STANDARD;

	App->render->camera.x = App->render->camera.y = 0;

	return true;
}

// Called each loop iteration
bool CreditScene::PreUpdate()
{

	return true;
}

// Called each loop iteration
bool CreditScene::Update(float dt)
{
	if (App->input->GetKey(SDL_SCANCODE_KP_5) == KEY_UP)
	{
		App->scene_manager->WantToChangeScene(DEV);
	}

	if (!credits_list.empty())
	{
		if (start_credit == false)
		{
			list<UIImage*>::iterator it = credits_list.begin();

			(*it)->SetVisible(true);
			App->ui->AnimFade((*it), 2, true, 2);

			credits_timer.Start();

			start_credit = true;
		}


		if (credits_timer.ReadSec() > 6)
		{
			list<UIImage*>::iterator it = credits_list.begin();

			(*it)->SetVisible(false);

			credits_list.pop_front();

			start_credit = false;
		}

	}

	else
	{
		credits->SetVisible(true);
	}
	
	return true;
}

// Called each loop iteration
bool CreditScene::PostUpdate()
{
	bool ret = true;

	if (close_game == true)
		return false;

	return ret;
}

// Called before quitting
bool CreditScene::CleanUp()
{
	LOG("Freeing Credit Scene");
	

	credits_list.clear();
	
	project = NULL;
	sheep_logo = NULL;
	upc_logo = NULL;
	credits = NULL;
	license_blizzard = NULL;
	license_image = NULL;

	return true;
}