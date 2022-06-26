#include <iostream>
#include <cassert>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "state.hpp"
#include "render.hpp"
#include "base.hpp"

int main(int argc, const char **argv)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
	{
		std::cerr << "Unable to initialize SDL2: " << SDL_GetError() << std::endl;
		return 1;
	}

	int img_flags = IMG_INIT_PNG | IMG_INIT_JPG;
	if ((IMG_Init(img_flags) & img_flags) != img_flags)
	{
		std::cerr << "Unable to initialize SDL2_image: " << IMG_GetError() << std::endl;
		return 1;
	}

	assert(argc >= 2);
	bool with_texture = argc > 2;

	texture texture;
	if (with_texture) assert(texture.load_from_file(argv[2]));

	mesh mesh;
	assert(mesh.load_from_file(argv[1], with_texture));

	SDL_Window *window = SDL_CreateWindow("gl3d", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	GlRender render(renderer);
	GlState state(mesh, with_texture ? &texture : nullptr);
	bool running = true;

	const float freq = SDL_GetPerformanceFrequency();
	const float frame_delta = 1000.0f / 60.0f;

	Uint64 last_time = SDL_GetPerformanceCounter();
	while (running)
	{
		Uint64 current_time = SDL_GetPerformanceCounter();
		float delta = (current_time - last_time) / freq * 1000.0f;

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					running = false;
					break;

				case SDL_KEYDOWN:
					state.keypress(event.key, delta);
					break;

				default:
					break;
			}
		}

		if (delta > frame_delta)
		{
			render.start_frame();
			render.clear({18, 18, 18, 255});
			state.update(render, delta);
			render.end_frame();

			last_time = current_time;
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	IMG_Quit();
	SDL_Quit();

	return 0;
}
