#pragma once

#include <SDL2/SDL.h>

#include "mat4.hpp"
#include "vec3.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "render.hpp"

class GlState
{
public:
	GlState(mesh &mesh, texture *texture = nullptr, float angle_factor = 0.0f, float fov = 90.0f, float near = 0.1f, float far = 1000.0f)
	: loaded_mesh(mesh), loaded_texture(texture), angle_factor(angle_factor), near_plane(near), far_plane(far)
	{
		const float aspect_ratio = (float)HEIGHT / (float)WIDTH;
		mat_proj = mat4::projection(fov, aspect_ratio, near, far);
		if (loaded_texture != nullptr)  texture_scale /= loaded_mesh.texture_max;
	}

	void update(GlRender &render, float delta);

	void keypress(SDL_KeyboardEvent &event, float delta);

private:
	mesh loaded_mesh;
	texture *loaded_texture;
	float texture_scale = 1.0f;

	float angle = 0;
	float angle_factor;

	vec3 camera{};
	vec3 look_dir = {0, 0, 1};
	float yaw = 0;

	mat4 mat_proj;
	float near_plane;
	float far_plane;
};
