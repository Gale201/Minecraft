#pragma once

#include <cmath>
#include <iostream>

#define PRINT_VEC3(v) std::cout << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
#define PRINT_VEC4(v) std::cout << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")\n";
#define PRINT_MAT4(mat) \
	std::cout << "[" << mat.m[0][0] << ", " << mat.m[0][1] << ", " << mat.m[0][2] << ", " << mat.m[0][3] << "]\n" \
			  << "[" << mat.m[1][0] << ", " << mat.m[1][1] << ", " << mat.m[1][2] << ", " << mat.m[1][3] << "]\n" \
			  << "[" << mat.m[2][0] << ", " << mat.m[2][1] << ", " << mat.m[2][2] << ", " << mat.m[2][3] << "]\n" \
			  << "[" << mat.m[3][0] << ", " << mat.m[3][1] << ", " << mat.m[3][2] << ", " << mat.m[3][3] << "]\n";


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

	Vec2f operator-() const
	{
		return Vec2f(-x, -y);
	}

	Vec2f operator*(float scalar) const
	{
		return Vec2f(x * scalar, y * scalar);
	}

	Vec2f& operator+=(const Vec2f& other)
	{
		x += other.x;
		y += other.y;
		return *this;
	}

	Vec2f& operator-=(const Vec2f& other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
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

	Vec3f operator-() const
	{ 
		return Vec3f(-x, -y, -z);
	}
	
	Vec3f operator*(float scalar) const
	{
		return Vec3f(x * scalar, y * scalar, z * scalar);
	}

	Vec3f& operator+=(const Vec3f& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	Vec3f& operator-=(const Vec3f& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	float length() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	Vec3f normalized() const
	{
		float len = length();
		if (len == 0.0f) return Vec3f(0, 0, 0);
		return Vec3f(x / len, y / len, z / len);
	}

	float Dot(const Vec3f& other) const
	{
		return x * other.x + y * other.y + z * other.z;
	}

	Vec3f Cross(const Vec3f& other) const
	{
		return {
			y * other.z - z * other.y,
			z * other.x - x * other.z,
			x * other.y - y * other.x
		};
	}
};

struct Mat4
{
	float m[4][4];

	Mat4()
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				m[i][j] = (i == j) ? 1.0f : 0.0f;
	}

	Mat4 operator*(const Mat4& other) const
	{
		Mat4 result;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				result.m[i][j] = 0.0f;
				for (int k = 0; k < 4; k++)
				{
					result.m[i][j] += m[i][k] * other.m[k][j];
				}
			}
		}
		return result;
	}

	static Mat4 Translate(const Vec3f& t)
	{
		Mat4 r;
		r.m[3][0] = t.x;
		r.m[3][1] = t.y;
		r.m[3][2] = t.z;
		return r;
	}

	static Mat4 Scale(const Vec3f& s)
	{
		Mat4 r;
		r.m[0][0] = s.x;
		r.m[1][1] = s.y;
		r.m[2][2] = s.z;
		return r;
	}

	static Mat4 RotateX(float angle)
	{
		Mat4 r;
		float c = std::cos(angle);
		float s = std::sin(angle);
		r.m[1][1] = c; r.m[1][2] = -s;
		r.m[2][1] = s; r.m[2][2] = c;
		return r;
	}

	static Mat4 RotateY(float angle)
	{
		Mat4 r;
		float c = std::cos(angle);
		float s = std::sin(angle);
		r.m[0][0] = c; r.m[0][2] = s;
		r.m[2][0] = -s; r.m[2][2] = c;
		return r;
	}

	static Mat4 RotateZ(float angle)
	{
		Mat4 r;
		float c = std::cos(angle);
		float s = std::sin(angle);
		r.m[0][0] = c; r.m[0][1] = -s;
		r.m[1][0] = s; r.m[1][1] = c;
		return r;
	}

	static Mat4 RotateXYZ(Vec3f angles)
	{
		return RotateX(angles.x) * RotateY(angles.y) * RotateZ(angles.z);
	}
};

struct Vec4f
{
	float x;
	float y;
	float z;
	float w;

	Vec4f() : x(0), y(0), z(0), w(0) {}
	Vec4f(float xx, float yy, float zz, float ww) : x(xx), y(yy), z(zz), w(ww) {}

	Vec4f operator+(const Vec4f& other) const
	{
		return Vec4f(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	Vec4f operator-(const Vec4f& other) const
	{
		return Vec4f(x - other.x, y - other.y, z - other.z, w - other.w);
	}

	Vec4f operator*(float scalar) const
	{
		return Vec4f(x * scalar, y * scalar, z * scalar, w * scalar);
	}

	Vec4f& operator+=(const Vec4f& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
		return *this;
	}

	Vec4f& operator-=(const Vec4f& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
		return *this;
	}

	Vec3f ToVec3f() const
	{
		return { x, y, z };
	}

	static Vec4f FromVec3f(Vec3f& v, float w)
	{
		return { v.x, v.y, v.z, w };
	}

	static Vec4f Transform(const Vec4f& v, const Mat4& m)
	{
		return Vec4f(
			v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + v.w * m.m[3][0],
			v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + v.w * m.m[3][1],
			v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + v.w * m.m[3][2],
			v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + v.w * m.m[3][3]
		);
	}
};
