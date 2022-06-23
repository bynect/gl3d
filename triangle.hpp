#pragma once

#include <ostream>
#include <SDL2/SDL.h>

#include "vec3.hpp"
#include "base.hpp"

struct triangle {
	vec3 vs[3] = {};
	SDL_Color color = {255, 255, 255, SDL_ALPHA_OPAQUE};

	friend std::ostream& operator<<(std::ostream &os, triangle &t)
	{
		return os << "triangle {" << t.vs[0] << ", " << t.vs[1] << ", " << t.vs[2] << "}";
	}

	static int clip_plane(vec3 plane_point, vec3 plane_normal, triangle &in, triangle &out1, triangle &out2);
};
