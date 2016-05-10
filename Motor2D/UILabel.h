#ifndef __UILABEL_H__
#define __UILABEL_H__

#include "UIEntity.h"
#include <string>

struct _TTF_Font;
struct SDL_Texture;

class UILabel : public UIEntity
{
public:

	UILabel();

	UILabel(const char* txt,const int x, const int y);

	// Destructor
	virtual ~UILabel();

	bool Update(float dt);

	void Print(string _text, bool isPassword = false);

	// Called before quitting
	bool CleanUp();
	
	string GetText(bool get_password = false) const;

private:

	string	text;
	string	password;

public:

	SDL_Texture* texture;

};

#endif