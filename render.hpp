#pragma once

#include <SDL2/SDL.h>
#include <vector>

#include "triangle.hpp"
#include "vec3.hpp"

class GlRender
{
public:
	GlRender(SDL_Renderer *renderer, size_t batch_size = 16384) : renderer(renderer)
	{
		points.reserve(batch_size);
	}

	void line(vec3 a, vec3 b, SDL_Color color)
	{
		color_set(color);

		SDL_RenderDrawLineF(renderer, a.x, a.y, b.x, b.y);
	}

	void triangle_frame(triangle t);

	// triangle scanline rasterization with top-left rule
	void triangle_filled(triangle t);

	void color_set(SDL_Color color)
	{
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	}

	void clear(SDL_Color color)
	{
		color_set(color);
		SDL_RenderClear(renderer);
	}

	void present()
	{
		SDL_RenderPresent(renderer);
	}

private:
	SDL_Renderer *renderer;
	std::vector<SDL_Point> points;

	void batch_add(int x, int y)
	{
		if (points.size() == points.capacity()) batch_flush();
		points.push_back({x, y});
	}

	void batch_flush()
	{
		SDL_RenderDrawPoints(renderer, points.data(), points.size());
		points.clear();
	}

	void triangle_bottom_flat(triangle t);

	void triangle_top_flat(triangle t);
};
