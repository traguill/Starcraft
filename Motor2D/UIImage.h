#ifndef __UIIMAGE_H__
#define __UIIMAGE_H__

#include "UIEntity.h"
struct SDL_Texture;

class UIImage : public UIEntity
{
public:

	UIImage();

	UIImage(const char* path, const int x, const int y);
	UIImage(SDL_Texture* tex, const int x, const int y);

	// Destructor
	virtual ~UIImage();

	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

	void LoadTexture(const char* path);

private:
	SDL_Texture* texture;

};

#endif