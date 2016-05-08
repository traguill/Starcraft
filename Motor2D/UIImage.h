#ifndef __UIIMAGE_H__
#define __UIIMAGE_H__

#include "UIEntity.h"
struct SDL_Texture;

class UIImage : public UIEntity
{
public:

	UIImage();

	UIImage(SDL_Rect section, const int x, const int y);

	// Destructor
	virtual ~UIImage();

	//For 90% of images
	bool Update(float dt);

	//For images that make more than one blit
	bool Draw();

	// Called before quitting
	bool CleanUp();

private:
	SDL_Rect section;
	iPoint init_pos;

};

#endif