#ifndef __INPUT_MANAGER_H__
#define __INPUT_MANAGER_H__

#include "j1Module.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1UIManager.h"
#include "PugiXml\src\pugixml.hpp"
#include <queue>

enum INPUT_TYPE
{
	K_DOWN,
	K_UP,
	K_REPEAT
};

struct ShortCut
{
	ShortCut()
	{}

	ShortCut(INPUT_TYPE _type, string _name, string _command)
	{
		type = _type;
		name = _name;
		command = _command;
	}

	INPUT_TYPE	 type;
	bool		 active;
	bool		 ready_to_change = false;
	string		 name;
	string		 command;

	UILabel*	 command_label = nullptr;
	UILabel*	 shortcut_label = nullptr;
};

class InputManager : public j1Module
{
public:

	InputManager();

	// Destructor
	virtual ~InputManager();

	// Called when before render is available
	bool Awake(pugi::xml_node&);

	// Call before first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	bool Update(float dt);

	// Called after all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	bool LoadShortcutsInfo();

	void ChangeShortcutCommand(ShortCut* shortcut);

public:
	list<ShortCut*>			shortcuts_list;

	string					inputs_file_path;

	const char*				new_command;

	list<string>		used_keys;

	bool					changing_command;

	void OnGUI(UIEntity* gui, GUI_EVENTS event);
};

#endif // __INPUT_MANAGER_H__