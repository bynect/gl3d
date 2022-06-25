#pragma once

#include <ostream>

struct vec2 {
	float u = 0, v = 0;
	float w = 1; // dummy

	friend std::ostream& operator<<(std::ostream &os, vec2 &v)
	{
		return os << "vec2 {" << v.u << ", " << v.v << ", " << "}";
	}
};
