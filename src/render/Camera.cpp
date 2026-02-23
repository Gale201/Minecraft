#include "Camera.h"

#include <cmath>

namespace Minecraft
{
	Mat4 Camera::GetViewMatrix() const
	{
		Vec3f worldUp{ 0.0f, 1.0f, 0.0f };
		Vec3f forward;
		forward.x = std::cosf(pitch) * std::sinf(yaw);
		forward.y = std::sinf(pitch);
		forward.z = std::cosf(pitch) * std::cosf(yaw);
		forward = forward.normalized();

		Vec3f right = -worldUp.Cross(forward).normalized();
		Vec3f up = -forward.Cross(right).normalized();

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
		r.m[2][2] = (nearPlane + farPlane) / (nearPlane - farPlane);
		r.m[2][3] = -1.0f;
		r.m[3][2] = (2.0f * nearPlane * farPlane) / (nearPlane - farPlane);
		r.m[3][3] = 0.0f;

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

	Frustum Camera::GetFrustum() const
	{
		Frustum f;

		const Mat4 viewProj = GetViewMatrix() * GetPerspectiveMatrix();

		// left
		f.planes[0].normal = {
			viewProj.m[0][3] + viewProj.m[0][0],
			viewProj.m[1][3] + viewProj.m[1][0],
			viewProj.m[2][3] + viewProj.m[2][0]
		};
		f.planes[0].d = viewProj.m[3][3] + viewProj.m[3][0];

		// right
		f.planes[1].normal = {
			viewProj.m[0][3] - viewProj.m[0][0],
			viewProj.m[1][3] - viewProj.m[1][0],
			viewProj.m[2][3] - viewProj.m[2][0]
		};
		f.planes[1].d = viewProj.m[3][3] - viewProj.m[3][0];

		// bottom
		f.planes[2].normal = {
			viewProj.m[0][3] + viewProj.m[0][1],
			viewProj.m[1][3] + viewProj.m[1][1],
			viewProj.m[2][3] + viewProj.m[2][1]
		};
		f.planes[2].d = viewProj.m[3][3] + viewProj.m[3][1];

		// top
		f.planes[3].normal = {
			viewProj.m[0][3] - viewProj.m[0][1],
			viewProj.m[1][3] - viewProj.m[1][1],
			viewProj.m[2][3] - viewProj.m[2][1]
		};
		f.planes[3].d = viewProj.m[3][3] - viewProj.m[3][1];

		// near
		f.planes[4].normal = {
			viewProj.m[0][3] + viewProj.m[0][2],
			viewProj.m[1][3] + viewProj.m[1][2],
			viewProj.m[2][3] + viewProj.m[2][2]
		};
		f.planes[4].d = viewProj.m[3][3] + viewProj.m[3][2];

		// far
		f.planes[5].normal = {
			viewProj.m[0][3] - viewProj.m[0][2],
			viewProj.m[1][3] - viewProj.m[1][2],
			viewProj.m[2][3] - viewProj.m[2][2]
		};
		f.planes[5].d = viewProj.m[3][3] - viewProj.m[3][2];

		for (int i = 0; i < 6; i++)
		{
			float length = f.planes[i].normal.length();
			f.planes[i].normal = f.planes[i].normal * (1 / length);
			f.planes[i].d = f.planes[i].d / length;
		}

		return f;
	}
}
