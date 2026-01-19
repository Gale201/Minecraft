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

inline void Renderer::DrawPixel(int x, int y, uint32_t color)
{
	if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) return;

	m_FrameBuffer[y * m_Width + x] = color;
}

inline void Renderer::DrawDepthPixel(int x, int y, float z, uint32_t color)
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

void Renderer::DrawTriangleWireframe(const Triangle& triangle, uint32_t color)
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

			// Draw if the pixel is inside the triangle and if its on the edge
			if (w0 >= 0 && w1 >= 0 && w2 >= 0 && (w0 <= 0.01f || w1 <= 0.01f || w2 <= 0.01f))
			{
				float z = w0 * triangle.v0.position.z + w1 * triangle.v1.position.z + w2 * triangle.v2.position.z;
				DrawDepthPixel(x, y, z - 0.01, color);
			}
		}
	}
}

void Renderer::DrawRenderObject(const RenderObject& renderObject, const Camera& camera)
{
	Mat4 modelMatrix = renderObject.transform.GetMatrix();
	Mat4 viewMatrix = camera.GetViewMatrix();
	Mat4 perspectiveMatrix = camera.GetPerspectiveMatrix();

	for (const Mesh& mesh : renderObject.model.meshes)
	{
		Triangle triangle;
		for (size_t i = 0; i < mesh.indices.size(); i += 3)
		{
			for (size_t j = 0; j < 3; j++)
			{
				// Get the model vertex from the mesh
				const Vertex& v = mesh.vertices[mesh.indices[i + j]];

				// Transform vertex from model space to world space
				Vec4f worldVertex = Vec4f::Transform(Vec4f(v.position.x, v.position.y, v.position.z, 1.0f), modelMatrix);

				// Transform world vertex to camera space
				Vec4f cameraVertex = Vec4f::Transform(worldVertex, viewMatrix);

				triangle.SetVertexByIndex(j, { cameraVertex.ToVec3f(), v.normal, v.u, v.v});
			}

			// Back face culling (happening in camera space)
			if (IsTriangleBackFacing(triangle))
				continue;

			// Near plane clipping (happening in camera space)
			Triangle clipped[2];
			int nClipped = ClipTriangleNearPlane(triangle, clipped, camera.nearPlane);
			
			for (size_t c = 0; c < nClipped; c++)
			{
				for (int j = 0; j < 3; j++)
				{
					// Clipped vertex
					Vertex clippedVertex = clipped[c].GetVertexByIndex(j);

					// Transform the clipped vertex to clip space
					Vec4f perspectiveVertex = Vec4f::Transform(Vec4f::FromVec3f(clippedVertex.position, 1.0f), perspectiveMatrix);

					// Perspective divide
					perspectiveVertex.x /= perspectiveVertex.w;
					perspectiveVertex.y /= perspectiveVertex.w;
					perspectiveVertex.z /= perspectiveVertex.w;

					Vertex screenVertex{ {}, clippedVertex.normal, clippedVertex.u, clippedVertex.v };

					// Convert to screen space
					screenVertex.position.x = (perspectiveVertex.x * 0.5f + 0.5f) * m_Width;
					screenVertex.position.y = (1.0f - (perspectiveVertex.y * 0.5f + 0.5f)) * m_Height;
					screenVertex.position.z = perspectiveVertex.z;

					triangle.SetVertexByIndex(j, screenVertex);
				}

				DrawTriangle(triangle, renderObject.color);
				//DrawTriangleWireframe(triangle, 0xFF0000FF);
			}
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

bool Renderer::IsTriangleBackFacing(const Triangle& triangle)
{
	Vec3f e1 = triangle.v1.position - triangle.v0.position;
	Vec3f e2 = triangle.v2.position - triangle.v0.position;

	Vec3f normal = e1.Cross(e2);
	
	return normal.Dot(-triangle.v0.position) > 0.0f;
}

int Renderer::ClipTriangleNearPlane(const Triangle& triangle, Triangle clipped[2], float nearPlane)
{
	clipped[0] = triangle;
	return 1;
}
