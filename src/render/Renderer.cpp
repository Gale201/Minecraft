#pragma once

#include <limits>
#include <algorithm>

#include "Renderer.h"
#include "../core/Log.h"

Renderer::~Renderer()
{
	delete[] m_FrameBuffer;
	delete[] m_DepthBuffer;
}

bool Renderer::Initialize(int width, int height)
{
	m_Width = width;
	m_Height = height;
	m_FrameBuffer = new uint32_t[width * height];
	m_DepthBuffer = new float[width * height];
	if (!m_FrameBuffer || !m_DepthBuffer)
		return false;
	return true;
}

void Renderer::Clear(uint32_t color)
{
	for (int i = 0; i < m_Width * m_Height; i++)
	{
		m_FrameBuffer[i] = color;
		m_DepthBuffer[i] = std::numeric_limits<float>::infinity();
	}
}

void Renderer::DrawPixel(int x, int y, uint32_t color)
{
	if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) return;

	m_FrameBuffer[y * m_Width + x] = color;
}

void Renderer::DrawDepthPixel(int x, int y, float z, uint32_t color)
{
	if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) return;

	if (z < m_DepthBuffer[y * m_Width + x])
	{
		m_DepthBuffer[y * m_Width + x] = z;
		m_FrameBuffer[y * m_Width + x] = color;
	}
}

void Renderer::DrawTriangle(const Triangle& triangle, uint32_t color)
{
	// Calculate the bound box
	int left, right, top, bottom;
	ComputeTriangleBoundingBox(triangle, left, right, top, bottom);

	// Clamp to screen bounds
	left = std::max(left, 0);
	right = std::min(right, m_Width - 1);
	top = std::max(top, 0);
	bottom = std::min(bottom, m_Height - 1);

	float divisor = (
		(triangle.v1.position.y - triangle.v2.position.y) * (triangle.v0.position.x - triangle.v2.position.x) +
		(triangle.v2.position.x - triangle.v1.position.x) * (triangle.v0.position.y - triangle.v2.position.y)
		);

	if (divisor == 0.0f) return;

	// Rasterize the triangle
	for (int y = top; y <= bottom; y++)
	{
		for (int x = left; x <= right; x++)
		{
			// Barycentric coordinates
			float w0 = (
				(triangle.v1.position.y - triangle.v2.position.y) * (x - triangle.v2.position.x) +
				(triangle.v2.position.x - triangle.v1.position.x) * (y - triangle.v2.position.y)
				) / divisor;

			float w1 = (
				(triangle.v2.position.y - triangle.v0.position.y) * (x - triangle.v2.position.x) +
				(triangle.v0.position.x - triangle.v2.position.x) * (y - triangle.v2.position.y)
				) / divisor;

			float w2 = 1.0f - w0 - w1;

			// If the pixel is inside the triangle
			if (w0 >= 0 && w1 >= 0 && w2 >= 0)
			{
				float z = w0 * triangle.v0.position.z + w1 * triangle.v1.position.z + w2 * triangle.v2.position.z;
				DrawDepthPixel(x, y, z, color);
			}
		}
	}
}

void Renderer::DrawRenderObject(const RenderObject& renderObject, const Camera& camera)
{
	Mat4 modelMatrix = renderObject.transform.GetMatrix();
	Mat4 viewMatrix = camera.GetViewMatrix();
	Mat4 projectionMatrix = camera.GetPerspectiveMatrix();
	Mat4 mvpMatrix = modelMatrix * viewMatrix * projectionMatrix;

	for (const Mesh& mesh : renderObject.model.meshes)
	{
		for (size_t i = 0; i < mesh.indices.size(); i += 3)
		{
			Triangle triangle;
			bool pointBehindCamera = false;

			for (size_t j = 0; j < 3; j++)
			{
				Vertex v = mesh.vertices[mesh.indices[i + j]];

				// Transform vertex
				Vec4f p = Vec4f::Transform(Vec4f(v.position.x, v.position.y, v.position.z, 1.0f), mvpMatrix);

				// Perspective divide
				if (p.w <= 0)
				{
					pointBehindCamera = true;
					break;
				}

				p.x /= p.w; 
				p.y /= p.w;
				p.z /= p.w;

				Vertex screenVertex;

				// Convert to screen space
				screenVertex.position.x = (p.x * 0.5f + 0.5f) * m_Width;
				screenVertex.position.y = (1.0f - (p.y * 0.5f + 0.5f)) * m_Height;
				screenVertex.position.z = p.z;

				triangle.SetVertexByIndex(j, screenVertex);
			}

			if (!pointBehindCamera)
				DrawTriangle(triangle, renderObject.color);
		}
	}
}

void Renderer::ComputeTriangleBoundingBox(const Triangle& triangle, int& left, int& right, int& top, int& bottom)
{
	left = static_cast<int>(std::min({ triangle.v0.position.x, triangle.v1.position.x, triangle.v2.position.x }));
	right = static_cast<int>(std::max({ triangle.v0.position.x, triangle.v1.position.x, triangle.v2.position.x }));
	top = static_cast<int>(std::min({ triangle.v0.position.y, triangle.v1.position.y, triangle.v2.position.y }));
	bottom = static_cast<int>(std::max({ triangle.v0.position.y, triangle.v1.position.y, triangle.v2.position.y }));
}

