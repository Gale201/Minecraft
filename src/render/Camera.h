#pragma once

#include <cmath>

#include "Frustum.h"
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

	Mat4 GetViewMatrix() const;
	Mat4 GetPerspectiveMatrix() const;
	
	Vec3f GetForward() const;
	Vec3f GetRight() const;

	Frustum GetFrustum() const;
};