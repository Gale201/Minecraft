#pragma once

#include <cmath>

#include "../math/Math.h"

struct Camera
{
	Vec3f position;
	float pitch;
	float yaw;
	float roll;
	float fovY;
	float aspect;
	float nearPlane;
	float farPlane;

	Mat4 GetViewMatrix() const
	{
		Mat4 translate = Mat4::Translate(Vec3f(-position.x, -position.y, -position.z));
		Mat4 rotate = Mat4::RotateXYZ(Vec3f(-pitch, -yaw, -roll));

		return rotate * translate;
	}

	Mat4 GetPerspectiveMatrix() const
	{
		float f = 1.0f / std::tan(fovY * 0.5f);
		
		Mat4 r;
		r.m[0][0] = f / aspect;
		r.m[1][1] = f;
		r.m[2][2] = farPlane / (nearPlane - farPlane);
		r.m[2][3] = -1.0f;
		r.m[3][2] = (nearPlane * farPlane) / (nearPlane - farPlane);

		return r;
	}
};