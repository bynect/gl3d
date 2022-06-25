#include <SDL2/SDL_image.h>
#include <iostream>
#include <cassert>

#include "texture.hpp"

bool texture::load_from_file(const char *path)
{
	surface = IMG_Load(path);

	if (surface == nullptr)
	{
		std::cerr << "Surface " << path << " not loaded: " << IMG_GetError() << std::endl;
		return false;
	}

	return true;
}

void texture::get_pixel(int x, int y, uint8_t &r, uint8_t &g, uint8_t &b) const
{
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	Uint32 pixel;
	switch (bpp)
	{
		case 1:
			pixel = *p;
			break;

		case 2:
			pixel = *(Uint16 *)p;
			break;

		case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			pixel = p[0] << 16 | p[1] << 8 | p[2];
#else
			pixel = p[0] | p[1] << 8 | p[2] << 16;
#endif
			break;

		case 4:
			pixel = *(Uint32 *)p;
			break;

		default:
			assert(false && "Unreachable");
	}

	SDL_GetRGB(pixel, surface->format, &r, &g, &b);
}
