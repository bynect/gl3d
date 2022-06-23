#include <cassert>

#include "triangle.hpp"
#include "vec3.hpp"

// plane_normal should be normalized
vec3 intersect_plane(vec3 &plane_point, vec3 &plane_normal, vec3 &line_start, vec3 &line_end)
{
	float plane_d = -(plane_normal.dot_product(plane_point));
	float ad = line_start.dot_product(plane_normal);
	float bd = line_end.dot_product(plane_normal);

	float t = (-plane_d - ad) / (bd - ad);
	vec3 line1 = line_end - line_start;
	vec3 line2 = line1 * t;
	return line_start + line2;
}

int triangle::clip_plane(vec3 plane_point, vec3 plane_normal, triangle &in, triangle &out1, triangle &out2)
{
	plane_normal = plane_normal.normalize();

	auto distance = [&](vec3 &p)
	{
		vec3 n = p.normalize();
		return plane_normal.x * p.x + plane_normal.y * p.y + plane_normal.z * p.z - plane_normal.dot_product(plane_point);
	};

	vec3 *inside[3];
	int inside_n = 0;

	vec3 *outside[3];
	int outside_n = 0;

	float d0 = distance(in.vs[0]);
	float d1 = distance(in.vs[1]);
	float d2 = distance(in.vs[2]);

	if (d0 >= 0) inside[inside_n++] = &in.vs[0];
	else outside[outside_n++] = &in.vs[0];

	if (d1 >= 0) inside[inside_n++] = &in.vs[1];
	else outside[outside_n++] = &in.vs[1];

	if (d2 >= 0) inside[inside_n++] = &in.vs[2];
	else outside[outside_n++] = &in.vs[2];

	switch (inside_n)
	{
		case 0:
			// All points lie outside of the plane
			return 0;

		case 3:
			// All points lie inside of the plane
			out1 = in;
			return 1;

		case 1:
			assert(outside_n == 2);

			out1.vs[0] = *inside[0];
			out1.vs[1] = intersect_plane(plane_point, plane_normal, *inside[0], *outside[0]);
			out1.vs[2] = intersect_plane(plane_point, plane_normal, *inside[0], *outside[1]);

			out1.color = in.color;
			return 1;

		case 2:
			assert(outside_n == 1);

			out1.vs[0] = *inside[0];
			out1.vs[1] = *inside[1];
			out1.vs[2] = intersect_plane(plane_point, plane_normal, *inside[0], *outside[0]);

			out2.vs[0] = *inside[1];
			out2.vs[1] = out1.vs[2];
			out2.vs[2] = intersect_plane(plane_point, plane_normal, *inside[1], *outside[0]);

			out1.color = in.color;
			out2.color = in.color;
			return 2;

		default:
			assert(false && "Unreachable");
	}
}
