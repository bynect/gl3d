#include <vector>
#include <cmath>
#include <iostream>

#include <SDL2/SDL.h>

#define WIDTH 800
#define HEIGHT 640

#define for_range(it, n, k) for (int it = n; it < k; it++)

using namespace std;

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

	void render(SDL_Renderer *renderer)
	{
		SDL_RenderDrawLineF(renderer, vs[0].x, vs[0].y, vs[1].x, vs[1].y);
		SDL_RenderDrawLineF(renderer, vs[0].x, vs[0].y, vs[2].x, vs[2].y);
		SDL_RenderDrawLineF(renderer, vs[2].x, vs[2].y, vs[1].x, vs[1].y);
	}

	friend ostream& operator<<(ostream &os, triangle &t)
	{
		return os << "triangle {" << t.vs[0] << ", " << t.vs[1] << ", " << t.vs[2] << "}";
	}
};

struct mesh {
	vector<triangle> ts;
};

float rad(float angle)
{
	const float PI = 3.14159265;
	return angle * PI / 180.0f;
}

class  GlState {
public:
	GlState() {
		cube_mesh.ts = {
			// south
			{ 0, 0, 0,   0, 1, 0,   1, 1, 0 },
			{ 0, 0, 0,   1, 1, 0,   1, 0, 0 },

			// east
			{ 1, 0, 0,   1, 1, 0,   1, 1, 1 },
			{ 1, 0, 0,   1, 1, 1,   1, 0, 1 },

			// north
			{ 1, 0, 1,   1, 1, 1,   0, 1, 1 },
			{ 1, 0, 1,   0, 1, 1,   0, 0, 1 },

			// west
			{ 0, 0, 1,   0, 1, 1,   0, 1, 0 },
			{ 0, 0, 1,   0, 1, 0,   0, 0, 0 },

			// up
			{ 0, 1, 0,   0, 1, 1,   1, 1, 1 },
			{ 0, 1, 0,   1, 1, 1,   1, 1, 0 },

			// bottom
			{ 1, 0, 1,   0, 0, 1,   0, 0, 0 },
			{ 1, 0, 1,   0, 0, 0,   1, 0, 0 },
		};


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

		for (auto t : cube_mesh.ts)
		{
			triangle rot1_t;
			for_range(i, 0, 3) rot1_t.vs[i] = multiply_matrix(t.vs[i], rot_z);

			triangle rot2_t;
			for_range(i, 0, 3) rot2_t.vs[i] = multiply_matrix(rot1_t.vs[i], rot_x);

			triangle trans_t = rot2_t;
			for_range(i, 0, 3) trans_t.vs[i].z += offset;

			triangle proj_t;
			for_range(i, 0, 3) proj_t.vs[i] = multiply_matrix(trans_t.vs[i], proj_matrix);

			for_range(i, 0, 3)
			{
				proj_t.vs[i].x = (proj_t.vs[i].x + 1) * 0.5f * WIDTH;
				proj_t.vs[i].y = (proj_t.vs[i].y + 1) * 0.5f * HEIGHT;
			}

			proj_t.render(renderer);
			//std::cout << proj_t << std::endl;

		}
		//std::cout << "delta = " << delta << ", angle = " << angle << std::endl;
	}

private:
	mesh cube_mesh{};
	mat4 proj_matrix{};

	float near = 0.1;
	float far = 1000;

	float fov = 90;
	float fov_rad = 1.0f / tanf(rad(fov * 0.5f));

	float aspect_ratio = (float)HEIGHT / (float)WIDTH;
	float offset = 3;
	float angle = 0;
};

int main()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
	{
		std::cerr << "Unable to initialize SDL2: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow("gl3d", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	GlState state;
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

			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
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
