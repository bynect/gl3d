#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <SDL2/SDL.h>

#define WIDTH 800
#define HEIGHT 640

#define for_range(it, n, k) for (int it = n; it < k; it++)

using namespace std;

float distance(float x, float y, float z)
{
	return sqrtf(x*x + y*y + z*z);
}

struct vec3 {
	float x, y, z;

	friend ostream& operator<<(ostream &os, vec3 &v)
	{
		return os << "vec3 {" << v.x << ", " << v.y << ", " << v.z << "}";
	}
};

struct mat4 {
	float m[4][4] = {};
};

vec3 multiply_matrix(vec3 &v, mat4 &m)
{
	vec3 _v = {
		.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0],
		.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1],
		.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2],
	};

	float w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + m.m[3][3];

	if (w != 0)
	{
		_v.x /= w;
		_v.y /= w;
		_v.z /= w;
	}

	return _v;
}

struct triangle {
	vec3 vs[3] = {};
	int color = 0xff;

	friend ostream& operator<<(ostream &os, triangle &t)
	{
		return os << "triangle {" << t.vs[0] << ", " << t.vs[1] << ", " << t.vs[2] << "}";
	}
};

void render_triangle(SDL_Renderer *renderer, triangle t)
{
	SDL_RenderDrawLineF(renderer, t.vs[0].x, t.vs[0].y, t.vs[1].x, t.vs[1].y);
	SDL_RenderDrawLineF(renderer, t.vs[0].x, t.vs[0].y, t.vs[2].x, t.vs[2].y);
	SDL_RenderDrawLineF(renderer, t.vs[2].x, t.vs[2].y, t.vs[1].x, t.vs[1].y);
}

float cross_product(vec3 a, vec3 b)
{
	return a.x * b.y - b.x * a.y;
}

void render_triangle_bottom_flat(SDL_Renderer *renderer, triangle t)
{
	float slope0 = (t.vs[1].x - t.vs[0].x) / (t.vs[1].y - t.vs[0].y);
	float slope1 = (t.vs[2].x - t.vs[0].x) / (t.vs[2].y - t.vs[0].y);

	const int y_min = (int)ceilf(t.vs[0].y - 0.5f);
	const int y_max = (int)ceilf(t.vs[2].y - 0.5f);

	for (int y = y_min; y < y_max; y++)
	{
		const float px0 = slope0 * ((float)y + 0.5f - t.vs[0].y) + t.vs[0].x;
		const float px1 = slope1 * ((float)y + 0.5f - t.vs[0].y) + t.vs[0].x;

		const int x_min = (int)ceilf(px0 - 0.5f);
		const int x_max = (int)ceilf(px1 - 0.5f);

		for (int x = x_min; x < x_max; x++)
		{
			SDL_RenderDrawPoint(renderer, x, y);
		}
	}
}

void render_triangle_top_flat(SDL_Renderer *renderer, triangle t)
{
	float slope0 = (t.vs[2].x - t.vs[0].x) / (t.vs[2].y - t.vs[0].y);
	float slope1 = (t.vs[2].x - t.vs[1].x) / (t.vs[2].y - t.vs[1].y);

	const int y_min = (int)ceilf(t.vs[0].y - 0.5f);
	const int y_max = (int)ceilf(t.vs[2].y - 0.5f);

	for (int y = y_min; y < y_max; y++)
	{
		const float px0 = slope0 * ((float)y + 0.5f - t.vs[0].y) + t.vs[0].x;
		const float px1 = slope1 * ((float)y + 0.5f - t.vs[1].y) + t.vs[1].x;

		const int x_min = (int)ceilf(px0 - 0.5f);
		const int x_max = (int)ceilf(px1 - 0.5f);

		for (int x = x_min; x < x_max; x++)
		{
			SDL_RenderDrawPoint(renderer, x, y);
		}
	}
}

// triangle scanline rasterization with top-left rule
void render_triangle_filled(SDL_Renderer *renderer, triangle t)
{
	SDL_SetRenderDrawColor(renderer, t.color, t.color, t.color, SDL_ALPHA_OPAQUE);

	if (t.vs[1].y < t.vs[0].y) swap(t.vs[1], t.vs[0]);
	if (t.vs[2].y < t.vs[0].y) swap(t.vs[2], t.vs[0]);
	if (t.vs[2].y < t.vs[1].y) swap(t.vs[2], t.vs[1]);

	if (t.vs[1].y == t.vs[2].y)
	{
		if (t.vs[2].x < t.vs[1].x) swap(t.vs[2], t.vs[1]);
		render_triangle_bottom_flat(renderer, t);
	}
	else if (t.vs[0].y == t.vs[1].y)
	{
		if (t.vs[1].x < t.vs[0].x) swap(t.vs[0], t.vs[1]);
		render_triangle_top_flat(renderer, t);
	}
	else
	{
		const float split = (t.vs[1].y - t.vs[0].y) / (t.vs[2].y - t.vs[0].y);
		const vec3 vi = {
			.x = t.vs[0].x + (t.vs[2].x - t.vs[0].x) * split,
			.y = t.vs[0].y + (t.vs[2].y - t.vs[0].y) * split,
			.z = 0,
		};

		triangle tmp1, tmp2;
		tmp1.color = tmp2.color = t.color;

		if (t.vs[1].x < vi.x)
		{
			// major right
			tmp1.vs[0] = t.vs[0];
			tmp1.vs[1] = t.vs[1];
			tmp1.vs[2] = vi;

			tmp2.vs[0] = t.vs[1];
			tmp2.vs[1] = vi;
			tmp2.vs[2] = t.vs[2];
		}
		else
		{
			// major left
			tmp1.vs[0] = t.vs[0];
			tmp1.vs[1] = vi;
			tmp1.vs[2] = t.vs[1];

			tmp2.vs[0] = vi;
			tmp2.vs[1] = t.vs[1];
			tmp2.vs[2] = t.vs[2];
		}

		render_triangle_bottom_flat(renderer, tmp1);
		render_triangle_top_flat(renderer, tmp2);
	}
}

struct mesh {
	vector<triangle> ts;

	bool load(const char *path)
	{
		ifstream f(path);
		if (!f.is_open()) return false;

		vector<vec3> verts;
		while (!f.eof())
		{
			char buf[256];
			f.getline(buf, 128);

			stringstream s;
			s << buf;

			char c;

			if (buf[0] == 'v')
			{
				vec3 v;
				s >> c >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}
			else if (buf[0] == 'f')
			{
				int face[3];
				s >> c >> face[0] >> face[1] >> face[2];
				ts.push_back({verts[face[0] - 1], verts[face[1] - 1], verts[face[2] - 1]});
			}
		}
		return true;
	}
};

float rad(float angle)
{
	const float PI = 3.14159265;
	return angle * PI / 180.0f;
}

class  GlState {
public:
	GlState(mesh &mesh, float offset = 8) : loaded_mesh(mesh), offset(offset)
	{
		proj_matrix.m[0][0] = aspect_ratio * fov_rad;
		proj_matrix.m[1][1] = fov_rad;
		proj_matrix.m[2][2] = far / (far - near);
		proj_matrix.m[2][3] = 1;
		proj_matrix.m[3][2] = (-far * near) / (far - near);
	}

	void update(SDL_Renderer *renderer, float delta)
	{
		// delta ms -> s
		angle += delta / 1000.0f;

		//std::cout << "delta = " << delta << ", angle = " << angle << std::endl;

		mat4 rot_z{};
		rot_z.m[0][0] = cosf(angle);
		rot_z.m[0][1] = sinf(angle);
		rot_z.m[1][0] = -sinf(angle);
		rot_z.m[1][1] = cosf(angle);
		rot_z.m[2][2] = 1;
		rot_z.m[3][3] = 1;

		mat4 rot_x{};
		rot_x.m[0][0] = 1;
		rot_x.m[1][1] = cosf(angle * 0.5f);
		rot_x.m[1][2] = sinf(angle * 0.5f);
		rot_x.m[2][1] = -sinf(angle * 0.5f);
		rot_x.m[2][2] = cosf(angle * 0.5f);
		rot_x.m[3][3] = 1;

		vector<triangle> raster_vec{};

		for (auto &t : loaded_mesh.ts)
		{
			triangle rot1_t;
			for_range(i, 0, 3) rot1_t.vs[i] = multiply_matrix(t.vs[i], rot_z);

			triangle rot2_t;
			for_range(i, 0, 3) rot2_t.vs[i] = multiply_matrix(rot1_t.vs[i], rot_x);

			triangle trans_t = rot2_t;
			for_range(i, 0, 3) trans_t.vs[i].z += offset;

			vec3 normal, line1, line2;
			line1.x = trans_t.vs[1].x - trans_t.vs[0].x;
			line1.y = trans_t.vs[1].y - trans_t.vs[0].y;
			line1.z = trans_t.vs[1].z - trans_t.vs[0].z;

			line2.x = trans_t.vs[2].x - trans_t.vs[0].x;
			line2.y = trans_t.vs[2].y - trans_t.vs[0].y;
			line2.z = trans_t.vs[2].z - trans_t.vs[0].z;

			normal.x = line1.y * line2.z - line1.z * line2.y;
			normal.y = line1.z * line2.x - line1.x * line2.z;
			normal.z = line1.x * line2.y - line1.y * line2.x;

			float l = distance(normal.x, normal.y, normal.z);
			normal.x /= l;
			normal.y /= l;
			normal.z /= l;

			if (normal.x * (trans_t.vs[0].x - camera.x) +
				normal.y * (trans_t.vs[0].y - camera.y) +
				normal.z * (trans_t.vs[0].z - camera.z) < 0)
			{
				vec3 light = {0, 0, -1};
				float l = distance(light.x, light.y, light.z);
				light.x /= l;
				light.y /= l;
				light.z /= l;

				float light_dp = normal.x * light.x + normal.y * light.y + normal.z * light.z;
				int color = 255 * light_dp;

				triangle proj_t;
				proj_t.color = color;
				for_range(i, 0, 3) proj_t.vs[i] = multiply_matrix(trans_t.vs[i], proj_matrix);

				for_range(i, 0, 3)
				{
					proj_t.vs[i].x = (proj_t.vs[i].x + 1) * 0.5f * WIDTH;
					proj_t.vs[i].y = (proj_t.vs[i].y + 1) * 0.5f * HEIGHT;
				}

				raster_vec.push_back(proj_t);
				//std::cout << proj_t << std::endl;
			}
		}

		sort(raster_vec.begin(), raster_vec.end(), [](triangle &t1, triangle &t2)
		{
			float z1 = (t1.vs[0].z + t1.vs[1].z + t1.vs[2].z) / 3.0f;
			float z2 = (t2.vs[0].z + t2.vs[1].z + t2.vs[2].z) / 3.0f;
			return z1 > z2;
		});

		for (auto &t : raster_vec)
		{
			render_triangle_filled(renderer, t);
			SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
			render_triangle(renderer, t);
		}
	}

private:
	mesh loaded_mesh{};
	mat4 proj_matrix{};

	vec3 camera{};
	float near = 0.1;
	float far = 1000;
	float angle = 0;

	float fov = 90;
	float fov_rad = 1.0f / tanf(rad(fov * 0.5f));

	float aspect_ratio = (float)HEIGHT / (float)WIDTH;
	float offset;
};

int main(int argc, const char **argv)
{
	mesh mesh;

	assert(argc == 2);
	assert(mesh.load(argv[1]));

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
	{
		std::cerr << "Unable to initialize SDL2: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow("gl3d", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	GlState state(mesh);
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

				default:
					break;
			}
		}

		if (delta > frame_delta)
		{
			SDL_SetRenderDrawColor(renderer, 18, 18, 18, 255);
			SDL_RenderClear(renderer);

			state.update(renderer, delta);
			SDL_RenderPresent(renderer);

			last_time = current_time;
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
