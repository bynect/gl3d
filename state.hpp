#pragma once

#include <SDL2/SDL.h>

#include "mat4.hpp"
#include "vec3.hpp"
#include "mesh.hpp"
#include "render.hpp"

class GlState
{
public:
	GlState(mesh &mesh, float fov = 90.0f, float near = 0.1f, float far = 1000.0f)
	: loaded_mesh(mesh), near_plane(near), far_plane(far)
	{
		const float aspect_ratio = (float)HEIGHT / (float)WIDTH;
		mat_proj = mat4::projection(fov, aspect_ratio, near, far);
	}

	void update(GlRender &render, float delta);

	void keypress(SDL_KeyboardEvent &event, float delta);

private:
	mesh loaded_mesh;
	mat4 mat_proj;
	float angle = 0;

	vec3 camera{};
	vec3 look_dir = {0, 0, 1};
	float yaw = 0;

	float near_plane;
	float far_plane;
};
