#include <cassert>
#include <deque>
#include <vector>
#include <algorithm>

#include "render.hpp"
#include "state.hpp"
#include "vec3.hpp"

void GlState::update(GlRender &render, float delta)
{
	// delta ms -> s
	angle += angle_factor * (delta / 1000.0f);
	//std::cout << "delta = " << delta << ", angle = " << angle << std::endl;

	auto mat_rot_z = mat4::rotation_z(angle * 0.5f);
	auto mat_rot_x = mat4::rotation_x(angle);

	auto mat_trans = mat4::translation(0.0f, 0.0f, 5.0f);
	auto mat_world = (mat_rot_z * mat_rot_x) * mat_trans;

	vec3 up_dir = {0, 1, 0};
	vec3 target_dir = {0, 0, 1};

	auto mat_camera_rot = mat4::rotation_y(yaw);
	look_dir = mat_camera_rot * target_dir;
	target_dir = camera + look_dir;

	auto mat_camera = mat4::point_at(camera, target_dir, up_dir);
	auto mat_view = mat_camera.quick_inverse();

	std::vector<triangle> raster_vec;
	for (auto &t : loaded_mesh.ts)
	{
		triangle trans_t;
		for_range(i, 0, 3) trans_t.vs[i] = mat_world * t.vs[i];

		auto line1 = trans_t.vs[1] - trans_t.vs[0];
		auto line2 = trans_t.vs[2] - trans_t.vs[0];

		auto normal = line1.cross_product(line2);
		normal = normal.normalize();

		auto camera_ray = trans_t.vs[0] - camera;
		if (normal.dot_product(camera_ray) < 0.0f)
		{
			// dynamic light position
			vec3 light = camera_ray * -1;
			light = light.normalize();

			float light_dp = std::max(0.1f, normal.dot_product(light));
			uint8_t greyscale = std::min(255.0f, (light_dp + 0.1f) * 255);

			triangle view_t;
			for_range(i, 0, 3) view_t.vs[i] = mat_view * trans_t.vs[i];

			// transfer texture information
			for_range(i, 0, 3) view_t.ts[i] = t.ts[i];

			triangle clipped[2];
			int clipped_n = triangle::clip_plane({0.0f, 0.0f, near_plane}, {0.0f, 0.0f, 1.0f}, view_t, clipped[0], clipped[1]);

			for_range(n, 0, clipped_n)
			{
				triangle proj_t;
				proj_t.color = {greyscale, greyscale, greyscale};

				for_range(i, 0, 3)
				{
					proj_t.vs[i] = mat_proj * clipped[n].vs[i];
					proj_t.ts[i] = clipped[n].ts[i];

					proj_t.ts[i].u /= proj_t.vs[i].w;
					proj_t.ts[i].v /= proj_t.vs[i].w;
					proj_t.ts[i].w = 1.0f / proj_t.vs[i].w;

					proj_t.vs[i] = proj_t.vs[i] / proj_t.vs[i].w;
				}

				for_range(i, 0, 3)
				{
					// Invert axis
					proj_t.vs[i].x *= -1.0f;
					proj_t.vs[i].y *= -1.0f;
				}

				vec3 offset = {1, 1, 0};
				for_range(i, 0, 3)
				{
					proj_t.vs[i] = proj_t.vs[i] + offset;
					proj_t.vs[i].x *= 0.5f * (float)WIDTH;
					proj_t.vs[i].y *= 0.5f * (float)HEIGHT;
				}

				raster_vec.push_back(proj_t);
				//std::cout << proj_t << std::endl;
			}
		}
	}

	//std::sort(raster_vec.begin(), raster_vec.end(), [](triangle &t1, triangle &t2)
	//{
	//	float z1 = (t1.vs[0].z + t1.vs[1].z + t1.vs[2].z) / 3.0f;
	//	float z2 = (t2.vs[0].z + t2.vs[1].z + t2.vs[2].z) / 3.0f;
	//	return z1 > z2;
	//});

	for (auto &prep_t : raster_vec)
	{
		std::deque<triangle> triangles;
		triangles.push_back(prep_t);

		int triangles_n = 1;
		for_range(plane, 0, 4)
		{
			triangle clipped[2];
			int clipped_n = 0;

			while (triangles_n > 0)
			{
				auto front = triangles.front();
				triangles.pop_front();
				triangles_n--;

				switch (plane)
				{
					case 0:
						clipped_n = triangle::clip_plane({0, 0, 0}, {0, 1, 0}, front, clipped[0], clipped[1]);
						break;

					case 1:
						clipped_n = triangle::clip_plane({0, (float)HEIGHT - 1, 0}, {0, -1, 0}, front, clipped[0], clipped[1]);
						break;

					case 2:
						clipped_n = triangle::clip_plane({0, 0, 0}, {1, 0, 0}, front, clipped[0], clipped[1]);
						break;

					case 3:
						clipped_n = triangle::clip_plane({(float)WIDTH - 1, 0, 0}, {-1, 0, 0}, front, clipped[0], clipped[1]);
						break;

					default:
						assert(false && "Unreachable");
				}

				for_range(n, 0, clipped_n) triangles.push_back(clipped[n]);
			}
			triangles_n = triangles.size();
		}

		for (auto &t : triangles)
		{
			if (loaded_texture != nullptr) render.triangle_textured(t, *loaded_texture, texture_scale);
			else render.triangle_filled(t);

			//t.color = {0, 255, 0};
			//render.triangle_frame(t);
		}
	}
}

void GlState::keypress(SDL_KeyboardEvent &event, float delta)
{
	const float camera_vel = 0.08f;
	const float yaw_vel = 0.02f;

	auto forward = look_dir * (camera_vel * delta);
	switch (event.keysym.sym)
	{
		case 'w':
			camera = camera + forward;
			break;

		case 'a':
			yaw -= yaw_vel * delta;
			break;

		case 's':
			camera = camera - forward;
			break;

		case 'd':
			yaw += yaw_vel * delta;
			break;

		case SDLK_UP:
			camera.y += camera_vel * delta;
			break;

		case SDLK_LEFT:
			// x is inverted
			camera.x += camera_vel * delta;
			break;

		case SDLK_DOWN:
			camera.y -= camera_vel * delta;
			break;

		case SDLK_RIGHT:
			// x is inverted
			camera.x -= camera_vel * delta;
			break;

		default:
			break;
	}
}
