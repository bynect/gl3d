#pragma once

#include <vector>

#include "triangle.hpp"

struct mesh {
	std::vector<triangle> ts;
	float texture_max = 1.0f;

	bool load_from_file(const char *path, bool with_texture = false);
};
