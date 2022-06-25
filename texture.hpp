#pragma once

#include <SDL2/SDL.h>

class texture
{
public:
	bool load_from_file(const char *path);

	void get_pixel(int x, int y, uint8_t &r, uint8_t &g, uint8_t &b) const;

	int get_width() const
	{
		return surface->w;
	}

	int get_height() const
	{
		return surface->h;
	}

private:
	SDL_Surface *surface;
};
