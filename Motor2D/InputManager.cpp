#include "InputManager.h"
#include "j1App.h"
#include "p2Defs.h"
#include "p2Log.h"
#include "UILabel.h"
#include "j1FileSystem.h"
#include "j1Input.h"

InputManager::InputManager() : j1Module()
{
	name.append("input_manager");
}

// Destructor
InputManager::~InputManager()
{}

// Called when before render is available
bool InputManager::Awake(pugi::xml_node& conf)
{
	bool ret = true;

	//Carregar Shortcuts info i guardar a shortcut_list
	inputs_file_path = conf.child("shortcuts_path").attribute("value").as_string();

	return ret;
}

// Call before first frame
bool InputManager::Start()
{
	bool ret = true;

	shortcuts_list.clear();

	LoadShortcutsInfo();

	return ret;
}

// Called before all Updates
bool InputManager::PreUpdate()
{
	bool ret = true;

	//TODO: Set active = true els shortcuts que coincideixin amb el contingut de la queue
	//DOWN
	if (!App->input->down_queue.empty())
	{
		for (int i = 0; i < App->input->down_queue.size(); i++)
		{
			list<ShortCut*>::iterator it = shortcuts_list.begin();
			while (it != shortcuts_list.end())
			{
				if (App->input->down_queue.front() + i == (*it)->command && (*it)->type == K_DOWN)
					(*it)->active = true;
				++it;
			}
		}
	}
	//UP
	if (!App->input->up_queue.empty())
	{
		for (int i = 0; i < App->input->up_queue.size(); i++)
		{
			list<ShortCut*>::iterator it = shortcuts_list.begin();
			while (it != shortcuts_list.end())
			{
				if (App->input->up_queue.front() + i == (*it)->command && (*it)->type == K_UP)
					(*it)->active = true;
				++it;
			}
		}
	}
	//REPEAT
	if (!App->input->repeat_queue.empty())
	{
		for (int i = 0; i < App->input->repeat_queue.size(); i++)
		{
			list<ShortCut*>::iterator it = shortcuts_list.begin();
			while (it != shortcuts_list.end())
			{
				if (App->input->repeat_queue.front() + i == (*it)->command && (*it)->type == K_REPEAT)
					(*it)->active = true;
				++it;
			}
		}
	}

	return ret;
}

bool InputManager::Update(float dt)
{
	bool ret = true;

	list<ShortCut*>::iterator it = shortcuts_list.begin();
	while (it != shortcuts_list.end())
	{
		if ((*it)->ready_to_change)
		{
			static SDL_Event event;

			SDL_WaitEvent(&event);

			if (event.type == SDL_KEYDOWN)
			{
				//NO ES COMPROBA SI M'ÉS D'UN SHORTCUT TENEN EL MATEIX COMMAND!!
				(*it)->command = SDL_GetScancodeName(event.key.keysym.scancode);
				ChangeShortcutCommand((*it));
				(*it)->ready_to_change = false;
			}
		}

		++it;
	}

	return ret;
}

// Called after all Updates
bool InputManager::PostUpdate()
{
	bool ret = true;

	//TODO: iterar shorcuts i posar els active a false
	list<ShortCut*>::iterator it = shortcuts_list.begin();

	while (it != shortcuts_list.end())
	{
		if ((*it)->active)
			(*it)->active = false;

		++it;
	}

	return ret;
}

// Called before quitting
bool InputManager::CleanUp()
{
	bool ret = true;

	shortcuts_list.clear();

	return ret;
}

bool InputManager::LoadShortcutsInfo()
{
	bool ret = true;

	//TODO: Load shortcuts info

	pugi::xml_document	inputs_data;
	pugi::xml_node		node;

	char* buf;
	int size = App->fs->Load(inputs_file_path.c_str(), &buf);
	pugi::xml_parse_result result = inputs_data.load_buffer(buf, size);
	delete[] buf;

	if (result == NULL)
	{
		LOG("Could not load xml file %s. PUGI error: &s", inputs_file_path.c_str(), result.description());
		return false;
	}
	else
		node = inputs_data.child("inputs_data");

	for (node = node.child("shortcut"); node && ret; node = node.next_sibling("shortcut"))
	{
		ShortCut* shortcut = new ShortCut();

		string type_tmp = node.child("TYPE").attribute("value").as_string();
		if (type_tmp == "UP")
			shortcut->type = K_UP;
		if (type_tmp == "DOWN")
			shortcut->type = K_DOWN;
		if (type_tmp == "REPEAT")
			shortcut->type = K_REPEAT;

		shortcut->name = node.child("name").attribute("value").as_string();
		shortcut->command = node.child("command").attribute("value").as_string();

		shortcut->active = false;

		shortcuts_list.push_back(shortcut);
	}

	return ret;
}

void InputManager::OnGUI(UIEntity* gui, GUI_EVENTS event)
{
	if (gui->type == LABEL)
	{
		list<ShortCut*>::iterator it = shortcuts_list.begin();

		while (it != shortcuts_list.end())
		{
			if (gui == (*it)->command_label && event == MOUSE_BUTTON_RIGHT_UP)
			{
				(*it)->ready_to_change = true;
			}
			++it;
		}

	}
}

void InputManager::ChangeShortcutCommand(ShortCut* shortcut)
{
	char n[20];
	sprintf_s(n, 20, "%c", shortcut->command);
	shortcut->command_label->SetText(n);

	shortcut->ready_to_change = true;
}