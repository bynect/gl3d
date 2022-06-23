#include <cstdint>
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

float rad(float angle)
{
	const float PI = 3.14159265;
	return angle * PI / 180.0f;
}

struct vec3 {
	float x = 0, y = 0, z = 0;
	float w = 1; // dummy

	friend ostream& operator<<(ostream &os, vec3 &v)
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
			y*v.z - z*v.y,
			z*v.x - x*v.z,
			x*v.y - y*v.x,
		};
	}
};

struct mat4 {
	float m[4][4] = {};

	vec3 operator*(vec3 &v)
	{
		return {
			.x = v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0] + v.w * m[3][0],
			.y = v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1] + v.w * m[3][1],
			.z = v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2] + v.w * m[3][2],
			.w = v.x * m[0][3] + v.y * m[1][3] + v.z * m[2][3] + v.w * m[3][3],
		};
	}

	mat4 operator*(mat4 &m2)
	{
		mat4 mat;
		for_range(i, 0, 4)
		{
			for_range(j, 0, 4)
			{
				mat.m[j][i] = m[j][0] * m2.m[0][i] + m[j][1] * m2.m[1][i] + m[j][2] * m2.m[2][i] + m[j][3] * m2.m[3][i];
			}
		}
		return mat;
	}

	static mat4 identity()
	{
		mat4 mat;
		mat.m[0][0] = 1.0f;
		mat.m[1][1] = 1.0f;
		mat.m[2][2] = 1.0f;
		mat.m[3][3] = 1.0f;
		return mat;
	}

	static mat4 rotation_x(float angle)
	{
		mat4 mat;
		mat.m[0][0] = 1.0f;
		mat.m[1][1] = cosf(angle);
		mat.m[1][2] = sinf(angle);
		mat.m[2][1] = -sinf(angle);
		mat.m[2][2] = cosf(angle);
		mat.m[3][3] = 1.0f;
		return mat;
	}

	static mat4 rotation_y(float angle)
	{
		mat4 mat;
		mat.m[0][0] = cosf(angle);
		mat.m[0][2] = sinf(angle);
		mat.m[2][0] = -sinf(angle);
		mat.m[1][1] = 1.0f;
		mat.m[2][2] = cosf(angle);
		mat.m[3][3] = 1.0f;
		return mat;
	}

	static mat4 rotation_z(float angle)
	{
		mat4 mat;
		mat.m[0][0] = cosf(angle);
		mat.m[0][1] = sinf(angle);
		mat.m[1][0] = -sinf(angle);
		mat.m[1][1] = cosf(angle);
		mat.m[2][2] = 1.0f;
		mat.m[3][3] = 1.0f;
		return mat;
	}

	static mat4 translation(float x, float y, float z)
	{
		mat4 mat;
		mat.m[0][0] = 1.0f;
		mat.m[1][1] = 1.0f;
		mat.m[2][2] = 1.0f;
		mat.m[3][3] = 1.0f;
		mat.m[3][0] = x;
		mat.m[3][1] = y;
		mat.m[3][2] = z;
		return mat;
	}

	static mat4 projection(float fov, float aspect_ratio, float near, float far)
	{
		float fov_rad = 1.0f / tanf(rad(fov * 0.5f));
		mat4 mat;
		mat.m[0][0] = aspect_ratio * fov_rad;
		mat.m[1][1] = fov_rad;
		mat.m[2][2] = far / (far - near);
		mat.m[2][3] = 1.0f;
		mat.m[3][2] = (-far * near) / (far - near);
		return mat;
	}

	static mat4 point_at(vec3 &pos, vec3 &target, vec3 &up)
	{
		auto forward = target - pos;
		forward = forward.normalize();

		auto a = forward * up.dot_product(forward);
		auto up2 = up - a;
		up2 = up2.normalize();

		auto right = up2.cross_product(forward);

		mat4 mat = {{
			{right.x, right.y, right.z, 0},
			{up2.x, up2.y, up2.z, 0},
			{forward.x, forward.y, forward.z, 0},
			{pos.x, pos.y, pos.z, 1},
		}};
		return mat;
	}

	// Works only for rotation/translation matrices
	mat4 quick_inverse()
	{
		mat4 mat;
		mat.m[0][0] = m[0][0];
		mat.m[0][1] = m[1][0];
		mat.m[0][2] = m[2][0];
		mat.m[0][3] = 0.0f;
		mat.m[1][0] = m[0][1];
		mat.m[1][1] = m[1][1];
		mat.m[1][2] = m[2][1];
		mat.m[1][3] = 0.0f;
		mat.m[2][0] = m[0][2];
		mat.m[2][1] = m[1][2];
		mat.m[2][2] = m[2][2];
		mat.m[2][3] = 0.0f;
		mat.m[3][0] = -(m[3][0] * mat.m[0][0] + m[3][1] * mat.m[1][0] + m[3][2] * mat.m[2][0]);
		mat.m[3][1] = -(m[3][0] * mat.m[0][1] + m[3][1] * mat.m[1][1] + m[3][2] * mat.m[2][1]);
		mat.m[3][2] = -(m[3][0] * mat.m[0][2] + m[3][1] * mat.m[1][2] + m[3][2] * mat.m[2][2]);
		mat.m[3][3] = 1.0f;
		return mat;
	}
};

struct triangle {
	vec3 vs[3] = {};
	SDL_Color color = {255, 255, 255, SDL_ALPHA_OPAQUE};

	friend ostream& operator<<(ostream &os, triangle &t)
	{
		return os << "triangle {" << t.vs[0] << ", " << t.vs[1] << ", " << t.vs[2] << "}";
	}
};

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
			f.getline(buf, 256);

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

class GlRender {
public:
	GlRender(SDL_Renderer *renderer, size_t batch_size = 16384) : renderer(renderer)
	{
		points.reserve(batch_size);
	}

	void triangle_frame(triangle t)
	{
		color_set(t.color);

		SDL_RenderDrawLineF(renderer, t.vs[0].x, t.vs[0].y, t.vs[1].x, t.vs[1].y);
		SDL_RenderDrawLineF(renderer, t.vs[0].x, t.vs[0].y, t.vs[2].x, t.vs[2].y);
		SDL_RenderDrawLineF(renderer, t.vs[2].x, t.vs[2].y, t.vs[1].x, t.vs[1].y);
	}

	// triangle scanline rasterization with top-left rule
	void triangle_filled(triangle t)
	{
		color_set(t.color);

		if (t.vs[1].y < t.vs[0].y) swap(t.vs[1], t.vs[0]);
		if (t.vs[2].y < t.vs[0].y) swap(t.vs[2], t.vs[0]);
		if (t.vs[2].y < t.vs[1].y) swap(t.vs[2], t.vs[1]);

		if (t.vs[1].y == t.vs[2].y)
		{
			if (t.vs[2].x < t.vs[1].x) swap(t.vs[2], t.vs[1]);
			triangle_bottom_flat(t);
		}
		else if (t.vs[0].y == t.vs[1].y)
		{
			if (t.vs[1].x < t.vs[0].x) swap(t.vs[0], t.vs[1]);
			triangle_top_flat(t);
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

			triangle_bottom_flat(tmp1);
			triangle_top_flat(tmp2);
		}
	}

	void color_set(SDL_Color color)
	{
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	}

	void clear(SDL_Color color)
	{
		color_set(color);
		SDL_RenderClear(renderer);
	}

	void present()
	{
		SDL_RenderPresent(renderer);
	}

private:
	SDL_Renderer *renderer;
	vector<SDL_Point> points;

	void batch_add(int x, int y)
	{
		if (points.size() == points.capacity()) batch_flush();
		points.push_back({x, y});
	}

	void batch_flush()
	{
	//	cout << "points " << points.size() << " of " << points.capacity() << endl;
		SDL_RenderDrawPoints(renderer, points.data(), points.size());
		points.clear();
	}

	void triangle_bottom_flat(triangle t)
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

			for (int x = x_min; x < x_max; x++) batch_add(x, y);
		}
		batch_flush();
	}

	void triangle_top_flat(triangle t)
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

			for (int x = x_min; x < x_max; x++) batch_add(x, y);
		}
		batch_flush();
	}
};

class  GlState {
public:
	GlState(mesh &mesh) : loaded_mesh(mesh)
	{
		mat_proj = mat4::projection(fov, aspect_ratio, near, far);
	}

	void update(GlRender &render, float delta)
	{
		// delta ms -> s
		//angle += delta / 1000.0f;
		//std::cout << "delta = " << delta << ", angle = " << angle << std::endl;

		auto mat_rot_z = mat4::rotation_z(angle * 0.5f);
		auto mat_rot_x = mat4::rotation_x(angle);

		auto mat_trans = mat4::translation(0.0f, 0.0f, 5.0f);
		auto mat_world = mat_rot_z * mat_rot_x * mat_trans;

		vec3 up_dir = {0, 1, 0};
		vec3 target_dir = {0, 0, 1};

		auto mat_camera_rot = mat4::rotation_y(yaw);
		look_dir = mat_camera_rot * target_dir;
		target_dir = camera + look_dir;

		auto mat_camera = mat4::point_at(camera, target_dir, up_dir);
		auto mat_view = mat_camera.quick_inverse();

		vector<triangle> raster_vec;
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
				vec3 light = {0, 1, -1};
				light = light.normalize();

				float light_dp = max(0.1f, light.dot_product(normal));
				uint8_t greyscale = 255 * light_dp;

				triangle view_t;
				for_range(i, 0, 3) view_t.vs[i] = mat_view * trans_t.vs[i];

				triangle proj_t;
				proj_t.color = {greyscale, greyscale, greyscale};

				for_range(i, 0, 3)
				{
					proj_t.vs[i] = mat_proj * view_t.vs[i];
					proj_t.vs[i] = proj_t.vs[i] / proj_t.vs[i].w;
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

		sort(raster_vec.begin(), raster_vec.end(), [](triangle &t1, triangle &t2)
		{
			float z1 = (t1.vs[0].z + t1.vs[1].z + t1.vs[2].z) / 3.0f;
			float z2 = (t2.vs[0].z + t2.vs[1].z + t2.vs[2].z) / 3.0f;
			return z1 > z2;
		});

		for (auto &t : raster_vec)
		{
			render.triangle_filled(t);
			//t.color = {0, 255, 0};
			//render.triangle_frame(t);
		}
	}

	void keypress(SDL_KeyboardEvent &event, float delta)
	{
		auto forward = look_dir * (0.08f * delta);
		switch (event.keysym.sym)
		{
			case 'w':
				camera.y += 0.08f * delta;
				break;

			case 'a':
				camera.x -= 0.08f * delta;
				break;

			case 's':
				camera.y -= 0.08f * delta;
				break;

			case 'd':
				camera.x += 0.08f * delta;
				break;

			case SDLK_UP:
				camera = camera + forward;
				break;

			case SDLK_LEFT:
				yaw -= 0.02f * delta;
				break;

			case SDLK_DOWN:
				camera = camera - forward;
				break;

			case SDLK_RIGHT:
				yaw += 0.02f * delta;
				break;

			default:
				break;
		}
	}

private:
	mesh loaded_mesh;
	mat4 mat_proj;

	vec3 camera;
	vec3 look_dir = {0, 0, 1};
	float yaw = 0;

	float near = 0.1;
	float far = 1000;
	float fov = 90;

	float aspect_ratio = (float)HEIGHT / (float)WIDTH;
	float angle = 0;
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
	GlRender render(renderer);
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

				case SDL_KEYDOWN:
					state.keypress(event.key, delta);
					break;

				default:
					break;
			}
		}

		if (delta > frame_delta)
		{
			render.clear({18, 18, 18, 255});
			state.update(render, delta);
			render.present();

			last_time = current_time;
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
