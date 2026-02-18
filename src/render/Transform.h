#pragma once
#include "../math/Math.h"

class Transform
{
public:
	Transform() : 
		m_Position{0.0f, 0.0f, 0.0f}, m_Rotation{0.0f, 0.0f, 0.0f}, m_Scale{1.0f, 1.0f, 1.0f}
	{
	}

	Transform(Vec3f position) :
		m_Position(position), m_Rotation{ 0.0f, 0.0f, 0.0f }, m_Scale{ 1.0f, 1.0f, 1.0f }
	{
	}
	
	~Transform() = default;

	const Mat4& GetMatrix() const
	{
		if (m_Update)
		{
			CalculateMatrix();
			m_Update = false;
		}
		return m_Matrix;
	}

	// Getters & Setters

	void SetPosition(const Vec3f& position) { 
		m_Position = position;
		m_Update = true;
	}

	void SetRotation(const Vec3f& rotation) { 
		m_Rotation = rotation; 
		m_Update = true;
	}

	void SetScale(const Vec3f& scale) { 
		m_Scale = scale; 
		m_Update = true;
	}

	void Translate(const Vec3f& delta) { 
		m_Position += delta; 
		m_Update = true;
	}

	void Rotate(const Vec3f& delta) { 
		m_Rotation += delta; 
		m_Update = true;
	}

	void Scale(const Vec3f& factor) {
		m_Scale = Vec3f(
			m_Scale.x * factor.x,
			m_Scale.y * factor.y,
			m_Scale.z * factor.z
		); 
		m_Update = true;
	}

	Vec3f GetPosition() const { return m_Position; }
	Vec3f GetRotation() const { return m_Rotation; }
	Vec3f GetScale() const { return m_Scale; }

private:
	void CalculateMatrix() const
	{
		Mat4 translate = Mat4::Translate(m_Position);
		Mat4 rotate = Mat4::RotateXYZ(m_Rotation);
		Mat4 scale = Mat4::Scale(m_Scale);
		m_Matrix = scale * rotate * translate;
	}

private:
	Vec3f m_Position;
	Vec3f m_Rotation;
	Vec3f m_Scale;
	mutable Mat4 m_Matrix;

	mutable bool m_Update = false;
};
