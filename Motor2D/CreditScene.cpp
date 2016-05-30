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
#include "j1Window.h"

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

	uint _w, _h;
	App->win->GetWindowSize(_w, _h);

	project = App->ui->CreateImage({ 1740, 536, 360, 70 }, 150, 200, false);
	sheep_logo = App->ui->CreateImage({ 2767, 10, 110, 110 }, _w / 2 - 55, _h / 2 - 55, false);
	upc_logo = App->ui->CreateImage({ 2883, 0, 356, 67 }, 150, 200, false);
	license_blizzard = App->ui->CreateImage({ 2795, 174, 528, 66 }, 60, 300, false);
	license_image = App->ui->CreateImage({ 2795, 244, 526, 77 }, 60, 300, false);

	credits_list.push_back(project);
	credits_list.push_back(upc_logo);
	credits_list.push_back(license_blizzard);
	credits_list.push_back(license_image);

	credits = App->ui->CreateImage({ 1806, 640, 304, 322 }, 160, _h, false);

	App->audio->PlayMusic("StarcraftTerranTheme2.wav");

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

		if (credits_timer.ReadSec() > 5)
		{
			list<UIImage*>::iterator it = credits_list.begin();
			App->ui->AnimFade((*it), 2, false, 2);
		}

		if (credits_timer.ReadSec() > 7)
		{
			list<UIImage*>::iterator it = credits_list.begin();

			//(*it)->SetVisible(false);

			credits_list.pop_front();

			start_credit = false;
		}

	}

	else
	{
		if (!finished_credits)
		{
			credits->SetVisible(true);

			if (start_credit == false)
			{

				credits_timer.Start();
				start_credit = true;

			}

			if (credits_timer.Read() > 1)
			{
				int _x, _y;
				credits->GetLocalPos(_x, _y);

				if (_y + credits->GetScreenRect().h > 0)
				{
					_y--;
					credits->SetLocalPos(_x, _y);
				}
				else
				{
					finished_credits = true;
				}

				start_credit = false;
			}
		}

		else
		{
			if (start_credit == false)
			{

				credits_timer.Start();
				start_credit = true;

			}

			if (fade_in_once)
			{

				App->ui->AnimFade(sheep_logo, 2, true, 1);
				fade_in_once = false;
			}

			if (credits_timer.ReadSec() > 6)
			{
				App->scene_manager->WantToChangeScene(MENU);
			}
		}

	}

	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
		App->scene_manager->WantToChangeScene(MENU);

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
