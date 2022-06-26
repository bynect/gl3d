#include <iostream>
#include <cassert>

#include "render.hpp"
#include "texture.hpp"
#include "triangle.hpp"

void GlRender::triangle_frame(triangle t)
{
	set_color(t.color);

	SDL_RenderDrawLineF(renderer, t.vs[0].x, t.vs[0].y, t.vs[1].x, t.vs[1].y);
	SDL_RenderDrawLineF(renderer, t.vs[0].x, t.vs[0].y, t.vs[2].x, t.vs[2].y);
	SDL_RenderDrawLineF(renderer, t.vs[2].x, t.vs[2].y, t.vs[1].x, t.vs[1].y);
}

void GlRender::triangle_textured(triangle t, const texture &texture, float texture_scale)
{
	if (t.vs[1].y < t.vs[0].y)
	{
		std::swap(t.vs[1], t.vs[0]);
		std::swap(t.ts[1], t.ts[0]);
	}

	if (t.vs[2].y < t.vs[0].y)
	{
		std::swap(t.vs[0], t.vs[2]);
		std::swap(t.ts[0], t.ts[2]);
	}

	if (t.vs[2].y < t.vs[1].y)
	{
		std::swap(t.vs[1], t.vs[2]);
		std::swap(t.ts[1], t.ts[2]);
	}

	float dy1 = t.vs[1].y - t.vs[0].y;
	float dx1 = t.vs[1].x - t.vs[0].x;

	float du1 = t.ts[1].u - t.ts[0].u;
	float dv1 = t.ts[1].v - t.ts[0].v;
	float dw1 = t.ts[1].w - t.ts[0].w;

	float dy2 = t.vs[2].y - t.vs[0].y;
	float dx2 = t.vs[2].x - t.vs[0].x;

	float du2 = t.ts[2].u - t.ts[0].u;
	float dv2 = t.ts[2].v - t.ts[0].v;
	float dw2 = t.ts[2].w - t.ts[0].w;

	float du1_step = 0, du2_step = 0;
	float dv1_step = 0, dv2_step = 0;
	float dw1_step = 0, dw2_step = 0;

	float slope0 = 0, slope1 = 0;
	if (dy1) slope0 = dx1 / fabs(dy1);
	if (dy2) slope1 = dx2 / fabs(dy2);

	if (dy1) du1_step = du1 / fabs(dy1);
	if (dy1) dv1_step = dv1 / fabs(dy1);
	if (dy1) dw1_step = dw1 / fabs(dy1);

	if (dy2) du2_step = du2 / fabs(dy2);
	if (dy2) dv2_step = dv2 / fabs(dy2);
	if (dy2) dw2_step = dw2 / fabs(dy2);

	float t_u, t_v, t_w;
	if (dy1)
	{
		const int y_min = (int)ceilf(t.vs[0].y - 0.5f);
		const int y_max = (int)ceilf(t.vs[1].y - 0.5f);

		for (int i = y_min; i < y_max; i++)
		{
			float ax = slope0 * ((float)i + 0.5f - t.vs[0].y) + t.vs[0].x;
			float bx = slope1 * ((float)i + 0.5f - t.vs[0].y) + t.vs[0].x;

			float t_su = t.ts[0].u + (float)(i - t.vs[0].y) * du1_step;
			float t_sv = t.ts[0].v + (float)(i - t.vs[0].y) * dv1_step;
			float t_sw = t.ts[0].w + (float)(i - t.vs[0].y) * dw1_step;

			float t_eu = t.ts[0].u + (float)(i - t.vs[0].y) * du2_step;
			float t_ev = t.ts[0].v + (float)(i - t.vs[0].y) * dv2_step;
			float t_ew = t.ts[0].w + (float)(i - t.vs[0].y) * dw2_step;

			if (ax > bx)
			{
				std::swap(ax, bx);
				std::swap(t_su, t_eu);
				std::swap(t_sv, t_ev);
				std::swap(t_sw, t_ew);
			}

			t_u = t_su;
			t_v = t_sv;
			t_w = t_sw;

			float tstep = 1.0f / ((float)(bx - ax));
			float t = 0.0f;

			const int x_min = (int)ceilf(ax - 0.5f);
			const int x_max = (int)ceilf(bx - 0.5f);

			for (int j = x_min; j < x_max; j++)
			{
				t_u = (1.0f - t) * t_su + t * t_eu;
				t_v = (1.0f - t) * t_sv + t * t_ev;
				t_w = (1.0f - t) * t_sw + t * t_ew;

				if (t_w > depth_buffer[i * WIDTH + j])
				{
					float t_x = clamp(t_u / t_w * texture_scale, 1.0f, 0.0f);
					float t_y = clamp(1.0f - (t_v / t_w * texture_scale), 1.0f, 0.0f);

					int x = (int)ceilf(t_x * texture.width());
					int y = (int)ceilf(t_y * texture.height());

					uint8_t r, g, b;
					texture.get_pixel(x, y, r, g, b);

					set_color({r, g, b});
					SDL_RenderDrawPoint(renderer, j, i);
					depth_buffer[i * WIDTH + j] = t_w;
				}
				t += tstep;
			}
		}
	}

	dy1 = t.vs[2].y - t.vs[1].y;
	dx1 = t.vs[2].x - t.vs[1].x;

	du1 = t.ts[2].u - t.ts[1].u;
	dv1 = t.ts[2].v - t.ts[1].v;
	dw1 = t.ts[2].w - t.ts[1].w;

	if (dy1) slope0 = dx1 / fabs(dy1);
	if (dy2) slope1 = dx2 / fabs(dy2);

	du1_step = 0, dv1_step = 0;
	if (dy1) du1_step = du1 / fabs(dy1);
	if (dy1) dv1_step = dv1 / fabs(dy1);
	if (dy1) dw1_step = dw1 / fabs(dy1);

	if (dy1)
	{
		const int y_min = (int)ceilf(t.vs[1].y - 0.5f);
		const int y_max = (int)ceilf(t.vs[2].y - 0.5f);

		for (int i = y_min; i < y_max; i++)
		{
			float ax = slope0 * ((float)i + 0.5f - t.vs[1].y) + t.vs[1].x;
			float bx = slope1 * ((float)i + 0.5f - t.vs[0].y) + t.vs[0].x;

			float t_su = t.ts[1].u + (float)(i - t.vs[1].y) * du1_step;
			float t_sv = t.ts[1].v + (float)(i - t.vs[1].y) * dv1_step;
			float t_sw = t.ts[1].w + (float)(i - t.vs[1].y) * dw1_step;

			float t_eu = t.ts[0].u + (float)(i - t.vs[0].y) * du2_step;
			float t_ev = t.ts[0].v + (float)(i - t.vs[0].y) * dv2_step;
			float t_ew = t.ts[0].w + (float)(i - t.vs[0].y) * dw2_step;

			if (ax > bx)
			{
				std::swap(ax, bx);
				std::swap(t_su, t_eu);
				std::swap(t_sv, t_ev);
				std::swap(t_sw, t_ew);
			}

			t_u = t_su;
			t_v = t_sv;
			t_w = t_sw;

			float tstep = 1.0f / ((float)(bx - ax));
			float t = 0.0f;

			const int x_min = (int)ceilf(ax - 0.5f);
			const int x_max = (int)ceilf(bx - 0.5f);

			for (int j = x_min; j < x_max; j++)
			{
				t_u = (1.0f - t) * t_su + t * t_eu;
				t_v = (1.0f - t) * t_sv + t * t_ev;
				t_w = (1.0f - t) * t_sw + t * t_ew;

				if (t_w > depth_buffer[i * WIDTH + j])
				{
					float t_x = clamp(t_u / t_w * texture_scale, 1.0f, 0.0f);
					float t_y = clamp(1.0f - (t_v / t_w * texture_scale), 1.0f, 0.0f);

					int x = (int)ceilf(t_x * texture.width());
					int y = (int)ceilf(t_y * texture.height());

					uint8_t r, g, b;
					texture.get_pixel(x, y, r, g, b);

					set_color({r, g, b});
					SDL_RenderDrawPoint(renderer, j, i);
					depth_buffer[i * WIDTH + j] = t_w;
				}
				t += tstep;
			}
		}
	}
}

// triangle scanline rasterization with top-left rule
void GlRender::triangle_filled(triangle t)
{
	set_color(t.color);

	if (t.vs[1].y < t.vs[0].y) std::swap(t.vs[1], t.vs[0]);
	if (t.vs[2].y < t.vs[0].y) std::swap(t.vs[2], t.vs[0]);
	if (t.vs[2].y < t.vs[1].y) std::swap(t.vs[2], t.vs[1]);

	if (t.vs[1].y == t.vs[2].y)
	{
		if (t.vs[2].x < t.vs[1].x) std::swap(t.vs[2], t.vs[1]);
		triangle_bottom_flat(t);
	}
	else if (t.vs[0].y == t.vs[1].y)
	{
		if (t.vs[1].x < t.vs[0].x) std::swap(t.vs[0], t.vs[1]);
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
	batch_flush();
}

void GlRender::triangle_bottom_flat(triangle t)
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
}

void GlRender::triangle_top_flat(triangle t)
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
}
