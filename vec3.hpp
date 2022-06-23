#pragma once

#include <ostream>
#include <cmath>

struct vec3 {
	float x = 0, y = 0, z = 0;
	float w = 1; // dummy

	friend std::ostream& operator<<(std::ostream &os, vec3 &v)
	{
		return os << "vec3 {" << v.x << ", " << v.y << ", " << v.z << "}";
	}

	vec3 operator+(vec3 &v)
	{
		return {x + v.x, y + v.y, z + v.z};
	}

	vec3 operator-(vec3 &v)
	{
		return {x - v.x, y - v.y, z - v.z};
	}

	vec3 operator*(float k)
	{
		return {x * k, y * k, z * k};
	}

	vec3 operator/(float k)
	{
		return {x / k, y / k, z / k};
	}

	vec3 operator*(vec3 &v)
	{
		return {x * v.x, y * v.y, z * v.z};
	}

	vec3 operator/(vec3 &v)
	{
		return {x / v.x, y / v.y, z / v.z};
	}

	float lenght()
	{
		return sqrtf(x*x + y*y + z*z);
	}

	vec3 normalize()
	{
		float l = lenght();
		return {x / l, y / l, z / l};
	}

	float dot_product(vec3 &v)
	{
		return x*v.x + y*v.y + z*v.z;
	}

	vec3 cross_product(vec3 &v)
	{
		return {
			.x = y*v.z - z*v.y,
			.y = z*v.x - x*v.z,
			.z = x*v.y - y*v.x,
		};
	}
};
