#pragma once

#include <vector>

#include "triangle.hpp"

struct mesh {
	std::vector<triangle> ts;

	bool load_from_file(const char *path);
};
