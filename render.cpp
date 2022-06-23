#include "render.hpp"

void GlRender::triangle_frame(triangle t)
{
	color_set(t.color);

	SDL_RenderDrawLineF(renderer, t.vs[0].x, t.vs[0].y, t.vs[1].x, t.vs[1].y);
	SDL_RenderDrawLineF(renderer, t.vs[0].x, t.vs[0].y, t.vs[2].x, t.vs[2].y);
	SDL_RenderDrawLineF(renderer, t.vs[2].x, t.vs[2].y, t.vs[1].x, t.vs[1].y);
}

// triangle scanline rasterization with top-left rule
void GlRender::triangle_filled(triangle t)
{
	color_set(t.color);

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
