#pragma once

#include <cmath>

#include "Frustum.h"
#include "../math/Math.h"

namespace Minecraft
{
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

		Mat4 GetViewMatrix() const;
		Mat4 GetPerspectiveMatrix() const;

		Vec3f GetForward() const;
		Vec3f GetRight() const;

		Frustum GetFrustum() const;
	};

	struct CameraConfig
	{
		float aspectRatio;  // fovY
		float fov;  // aspect
		float nearPlane;
		float farPlane;
	};
}
