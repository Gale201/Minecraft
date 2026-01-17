#pragma once
#include <cmath>

struct Vec2f
{
	float x;
	float y;

	Vec2f() : x(0), y(0) {}
	Vec2f(float xx, float yy) : x(xx), y(yy) {}

	Vec2f operator+(const Vec2f& other) const
	{
		return Vec2f(x + other.x, y + other.y);
	}

	Vec2f operator-(const Vec2f& other) const
	{
		return Vec2f(x - other.x, y - other.y);
	}

	Vec2f operator*(float scalar) const
	{
		return Vec2f(x * scalar, y * scalar);
	}
};

struct Vec3f
{
	float x;
	float y;
	float z;

	Vec3f() : x(0), y(0), z(0) {}
	Vec3f(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}

	Vec3f operator+(const Vec3f& other) const
	{
		return Vec3f(x + other.x, y + other.y, z + other.z);
	}
	
	Vec3f operator-(const Vec3f& other) const
	{
		return Vec3f(x - other.x, y - other.y, z - other.z);
	}
	
	Vec3f operator*(float scalar) const
	{
		return Vec3f(x * scalar, y * scalar, z * scalar);
	}

	float length() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	Vec3f normalized() const
	{
		float len = length();
		return Vec3f(x / len, y / len, z / len);
	}
};