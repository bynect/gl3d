#pragma once

#include <cmath>
#include "vec3.hpp"
#include "base.hpp"

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
		float fov_rad = 1.0f / tanf(deg2rad(fov * 0.5f));
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
