#include "Camera.h"

#include <cmath>

Mat4 Camera::GetViewMatrix() const
{
	Vec3f forward = -GetForward().normalized();

	Vec3f worldUp = { 0, 1, 0 };
	Vec3f right = -worldUp.Cross(forward).normalized();
	Vec3f up = -forward.Cross(right);

	Mat4 v;

	v.m[0][0] = right.x;   v.m[0][1] = up.x;   v.m[0][2] = forward.x;   v.m[0][3] = 0;
	v.m[1][0] = right.y;   v.m[1][1] = up.y;   v.m[1][2] = forward.y;   v.m[1][3] = 0;
	v.m[2][0] = right.z;   v.m[2][1] = up.z;   v.m[2][2] = forward.z;   v.m[2][3] = 0;

	v.m[3][0] = -right.Dot(position);
	v.m[3][1] = -up.Dot(position);
	v.m[3][2] = -forward.Dot(position);
	v.m[3][3] = 1.0f;

	return v;
}

Mat4 Camera::GetPerspectiveMatrix() const
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

Vec3f Camera::GetForward() const
{
	return {
		-std::cos(pitch) * std::sin(yaw),
		-std::sin(pitch),
		-std::cos(pitch) * std::cos(yaw)
	};
}

Vec3f Camera::GetRight() const
{
	return {
		-std::cos(yaw),
		0.0f,
		std::sin(yaw)
	};
}
