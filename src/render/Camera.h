#pragma once

#include <cmath>

#include "Frustum.h"
#include "../math/Math.h"

namespace Minecraft
{
	struct Camera
	{
		Vec3f position;
		float pitch{ 0.0f };
		float yaw{ 0.0f };
		float roll{ 0.0f };
		float fovY{ 0.0f };
		float aspect{ 0.0f };
		float nearPlane{ 0.0f };
		float farPlane{ 0.0f };

		Mat4 GetViewMatrix() const;
		Mat4 GetPerspectiveMatrix() const;

		Vec3f GetForward() const;
		Vec3f GetRight() const;

		Frustum GetFrustum() const;
	};

	struct CameraConfig
	{
		float aspectRatio{ 0.0f };  // fovY
		float fov{ 0.0f };  // aspect
		float nearPlane{ 0.0f };
		float farPlane{ 0.0f };
	};
}
