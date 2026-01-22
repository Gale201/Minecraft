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

void Renderer::DrawTriangleBarycentric(const Triangle& triangle, const Texture& texture)
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

				float invW = w0 * triangle.v0.invW + w1 * triangle.v1.invW + w2 * triangle.v2.invW;
				float u = (w0 * triangle.v0.u + w1 * triangle.v1.u + w2 * triangle.v2.u) / invW;
				float v = (w0 * triangle.v0.v + w1 * triangle.v1.v + w2 * triangle.v2.v) / invW;

				if (0)
				{
					DrawDepthPixel(x, y, z, texture.GetPixelAtUV(u, v));
				}
				else
				{
					Vec3f normal = (triangle.v0.normal * w0 + triangle.v1.normal * w1 + triangle.v2.normal * w2).normalized();
					uint32_t textureColor = texture.GetPixelAtUV(u, v);
					DrawDepthPixel(x, y, z, ComputeShadedColorForNormal(normal, m_DirectionalLight, m_AmbientLight, textureColor));
				}
			}
		}
	}
}

void Renderer::DrawTriangleScanline(const Triangle& triangle, const Texture& texture)
{
	Triangle sorted = SortTriangleVerticesByY(triangle);

	Vertex v0 = sorted.v0;
	Vertex v1 = sorted.v1;
	Vertex v2 = sorted.v2;

	if ((int)v1.position.y == (int)v2.position.y)
	{
		DrawFlatBottomTriangle(
			v0.position.x, v0.position.y, v0.position.z, v0.u, v0.v, v0.normal.x, v0.normal.y, v0.normal.z, v0.invW,
			v1.position.x, v1.position.y, v1.position.z, v1.u, v1.v, v1.normal.x, v1.normal.y, v1.normal.z, v1.invW,
			v2.position.x, v2.position.y, v2.position.z, v2.u, v2.v, v2.normal.x, v2.normal.y, v2.normal.z, v2.invW,
			texture
		);
	}
	else if ((int)v0.position.y == (int)v1.position.y)
	{
		DrawFlatTopTriangle(
			v0.position.x, v0.position.y, v0.position.z, v0.u, v0.v, v0.normal.x, v0.normal.y, v0.normal.z, v0.invW,
			v1.position.x, v1.position.y, v1.position.z, v1.u, v1.v, v1.normal.x, v1.normal.y, v1.normal.z, v1.invW,
			v2.position.x, v2.position.y, v2.position.z, v2.u, v2.v, v2.normal.x, v2.normal.y, v2.normal.z, v2.invW,
			texture
		);
	}
	else
	{
		float t = (v1.position.y - v0.position.y) / (v2.position.y - v0.position.y);
		Vertex vNew;
		vNew.position.x = v0.position.x + t * (v2.position.x - v0.position.x);
		vNew.position.y = v1.position.y;
		vNew.position.z = v0.position.z + t * (v2.position.z - v0.position.z);
		vNew.u = v0.u + t * (v2.u - v0.u);
		vNew.v = v0.v + t * (v2.v - v0.v);
		vNew.normal = v0.normal + (v2.normal - v0.normal) * t;

		vNew.invW = v0.invW + t * (v2.invW - v0.invW);

		DrawFlatBottomTriangle(
			v0.position.x, v0.position.y, v0.position.z, v0.u, v0.v, v0.normal.x, v0.normal.y, v0.normal.z, v0.invW,
			v1.position.x, v1.position.y, v1.position.z, v1.u, v1.v, v1.normal.x, v1.normal.y, v1.normal.z, v1.invW,
			vNew.position.x, vNew.position.y, vNew.position.z, vNew.u, vNew.v, vNew.normal.x, vNew.normal.y, vNew.normal.z, vNew.invW,
			texture
		);
		DrawFlatTopTriangle(
			v1.position.x, v1.position.y, v1.position.z, v1.u, v1.v, v1.normal.x, v1.normal.y, v1.normal.z, v1.invW,
			vNew.position.x, vNew.position.y, vNew.position.z, vNew.u, vNew.v, vNew.normal.x, vNew.normal.y, vNew.normal.z, vNew.invW,
			v2.position.x, v2.position.y, v2.position.z, v2.u, v2.v, v2.normal.x, v2.normal.y, v2.normal.z, v2.invW,
			texture
		);
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

				triangle.SetVertexByIndex(j, { cameraVertex.ToVec3f(), v.normal, v.u, v.v });
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

					clippedVertex.u /= perspectiveVertex.w;
					clippedVertex.v /= perspectiveVertex.w;

					Vertex screenVertex{ {}, clippedVertex.normal, clippedVertex.u, clippedVertex.v };

					// Convert to screen space
					screenVertex.position.x = (int) ((perspectiveVertex.x * 0.5f + 0.5f) * m_Width);
					screenVertex.position.y = (int) ((1.0f - (perspectiveVertex.y * 0.5f + 0.5f)) * m_Height);
					screenVertex.position.z = perspectiveVertex.z;
					screenVertex.invW = 1.0f / perspectiveVertex.w;

					triangle.SetVertexByIndex(j, screenVertex);
				}

				DrawTriangleBarycentric(triangle, renderObject.texture);
				// DrawTriangleWireframe(triangle, 0xFF0000FF);
			}
		}
	}
}

// private functions

Triangle Renderer::SortTriangleVerticesByY(const Triangle& triangle)
{
	Triangle sorted;
	sorted.v0 = triangle.v0;
	sorted.v1 = triangle.v1;
	sorted.v2 = triangle.v2;

	if (sorted.v0.position.y > sorted.v1.position.y) std::swap(sorted.v0, sorted.v1);
	if (sorted.v1.position.y > sorted.v2.position.y) std::swap(sorted.v1, sorted.v2);
	if (sorted.v0.position.y > sorted.v1.position.y) std::swap(sorted.v0, sorted.v1);

	return sorted;
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
	
	return normal.Dot(-triangle.v0.position) >= 0.0f && normal.z >= 0.0f;
}

Vertex Renderer::IntersectWithNearPlane(const Vertex& v0, const Vertex& v1, float nearPlane)
{
	// If the 2 points are very close on the z axis return the first point with z = -nearPlane
	if (std::fabsf(v1.position.z - v0.position.z) <= 1e-6f)
		return { { v1.position.x, -nearPlane, v1.position.y } };

	// Calculate the weight factor
	float t = (-nearPlane - v0.position.z) / (v1.position.z - v0.position.z);

	Vertex r;
	r.position.x = v0.position.x + t * (v1.position.x - v0.position.x);
	r.position.y = v0.position.y + t * (v1.position.y - v0.position.y);
	r.position.z = -nearPlane;
	r.u = v0.u + t * (v1.u - v0.u);
	r.v = v0.v + t * (v1.v - v0.v);
	r.normal = v0.normal + (v1.normal - v0.normal) * t;

	return r;
}

int Renderer::ClipTriangleNearPlane(const Triangle& triangle, Triangle clipped[2], float nearPlane)
{
	Vertex inside[3], outside[3];
	size_t countInside = 0, countOutside = 0;

	// Find which vertices are behind (inside) the near plane
	for (size_t i = 0; i < 3; i++)
	{
		const Vertex& v = triangle.GetVertexByIndex(i);
		if (v.position.z < -nearPlane)
			inside[countInside++] = v;
		else
			outside[countOutside++] = v;
	}

	// If there are 0 vertices behind the near plane then don't render anything
	if (countInside == 0)
		return 0;
	// If all vertices are behind then just render the triangle
	else if (countInside == 3)
	{
		clipped[0] = triangle;
		return 1;
	}
	// If there is 1 vertex inside, create 1 new triangle
	else if (countInside == 1)
	{
		Vertex vInside = inside[0], vOutside1 = outside[0], vOutside2 = outside[1];

		Vertex intersection1 = IntersectWithNearPlane(vInside, vOutside1, nearPlane);
		Vertex intersection2 = IntersectWithNearPlane(vInside, vOutside2, nearPlane);

		clipped[0] = { vInside, intersection1, intersection2 };
		return 1;
	}
	// If there are 2 vertices inside, create 2 new triangles
	else if (countInside == 2)
	{
		Vertex vInside1 = inside[0], vInside2 = inside[1], vOutside = outside[0];

		Vertex intersection1 = IntersectWithNearPlane(vInside1, vOutside, nearPlane);
		Vertex intersection2 = IntersectWithNearPlane(vInside2, vOutside, nearPlane);

		clipped[0] = { vInside1, vInside2, intersection1 };
		clipped[1] = { vInside2, intersection2, intersection1 };

		return 2;
	}

	return 0;
}

uint32_t Renderer::ComputeShadedColorForNormal(Vec3f normal, Vec3f lightDir, float ambientLight, uint32_t color)
{
	float intensity = normal.Dot(lightDir);
	intensity = std::fminf(std::fmaxf(intensity, 0.0f) + ambientLight, 1.0f);

	uint8_t a = (color >> 24) & 0xFF;
	uint8_t r = (color >> 16) & 0xFF;
	uint8_t g = (color >> 8) & 0xFF;
	uint8_t b = color & 0xFF;

	r = (uint8_t)(r * intensity);
	g = (uint8_t)(g * intensity);
	b = (uint8_t)(b * intensity);

	return (a << 24) | (r << 16) | (g << 8) | b;
}

void Renderer::DrawFlatBottomTriangle(
	float x0, float y0, float z0, float u0, float v0, float xn0, float yn0, float zn0, float invW0,
	float x1, float y1, float z1, float u1, float v1, float xn1, float yn1, float zn1, float invW1,
	float x2, float y2, float z2, float u2, float v2, float xn2, float yn2, float zn2, float invW2,
	const Texture& texture
)
{
	float slope_x1 = (x1 - x0) / (y1 - y0);
	float slope_x2 = (x2 - x0) / (y2 - y0);

	float slope_z1 = (z1 - z0) / (y1 - y0);
	float slope_z2 = (z2 - z0) / (y2 - y0);

	float slope_u1 = (u1 - u0) / (y1 - y0);
	float slope_u2 = (u2 - u0) / (y2 - y0);

	float slope_v1 = (v1 - v0) / (y1 - y0);
	float slope_v2 = (v2 - v0) / (y2 - y0);

	float slope_xn1 = (xn1 - xn0) / (y1 - y0);
	float slope_xn2 = (xn2 - xn0) / (y2 - y0);

	float slope_yn1 = (yn1 - yn0) / (y1 - y0);
	float slope_yn2 = (yn2 - yn0) / (y2 - y0);

	float slope_zn1 = (zn1 - zn0) / (y1 - y0);
	float slope_zn2 = (zn2 - zn0) / (y2 - y0);

	float slope_invW1 = (invW1 - invW0) / (y1 - y0);
	float slope_invW2 = (invW2 - invW0) / (y2 - y0);

	for (int y = y0; y < y1; y++)
	{
		if (y < 0 || y >= m_Height) continue;

		float py = y + 0.5f;
		float dy = py - y0;

		float xl = x0 + dy * slope_x1;
		float xr = x0 + dy * slope_x2;
		float zl = z0 + dy * slope_z1;
		float zr = z0 + dy * slope_z2;

		float ul = u0 + dy * slope_u1;
		float ur = u0 + dy * slope_u2;
		float vl = v0 + dy * slope_v1;
		float vr = v0 + dy * slope_v2;

		float xnl = xn0 + dy * slope_xn1;
		float xnr = xn0 + dy * slope_xn2;
		float ynl = yn0 + dy * slope_yn1;
		float ynr = yn0 + dy * slope_yn2;
		float znl = zn0 + dy * slope_zn1;
		float znr = zn0 + dy * slope_zn2;

		float invWl = invW0 + dy * slope_invW1;
		float invWr = invW0 + dy * slope_invW2;

		if (xl > xr)
		{
			std::swap(xl, xr);
			std::swap(zl, zr);
			std::swap(ul, ur);
			std::swap(vl, vr);
			std::swap(xnl, xnr);
			std::swap(ynl, ynr);
			std::swap(znl, znr);
			std::swap(invWl, invWr);
		}

		DrawHorizontalDepthTextureLine(xl, xr, y, zl, zr, ul, ur, vl, vr, xnl, xnr, ynl, ynr, znl, znr, invWl, invWr, texture);
	}
}

void Renderer::DrawFlatTopTriangle(
	float x0, float y0, float z0, float u0, float v0, float xn0, float yn0, float zn0, float invW0,
	float x1, float y1, float z1, float u1, float v1, float xn1, float yn1, float zn1, float invW1,
	float x2, float y2, float z2, float u2, float v2, float xn2, float yn2, float zn2, float invW2,
	const Texture& texture
)
{
	float slope_x1 = (x2 - x0) / (y2 - y0);
	float slope_x2 = (x2 - x1) / (y2 - y1);

	float slope_z1 = (z2 - z0) / (y2 - y0);
	float slope_z2 = (z2 - z1) / (y2 - y1);

	float slope_u1 = (u2 - u0) / (y2 - y0);
	float slope_u2 = (u2 - u1) / (y2 - y1);

	float slope_v1 = (v2 - v0) / (y2 - y0);
	float slope_v2 = (v2 - v1) / (y2 - y1);

	float slope_xn1 = (xn2 - xn0) / (y2 - y0);
	float slope_xn2 = (xn2 - xn1) / (y2 - y1);

	float slope_yn1 = (yn2 - yn0) / (y2 - y0);
	float slope_yn2 = (yn2 - yn1) / (y2 - y1);

	float slope_zn1 = (zn2 - zn0) / (y2 - y0);
	float slope_zn2 = (zn2 - zn1) / (y2 - y1);

	float slope_invW1 = (invW2 - invW0) / (y2 - y0);
	float slope_invW2 = (invW2 - invW1) / (y2 - y1);

	for (int y = y2 - 1; y >= y0; y--)
	{
		if (y < 0 || y >= m_Height) continue;

		float py = y + 0.5f;
		float dy0 = py - y0;
		float dy1 = py - y1;

		float xl = x0 + dy0 * slope_x1;
		float xr = x1 + dy1 * slope_x2;
		float zl = z0 + dy0 * slope_z1;
		float zr = z1 + dy1 * slope_z2;

		float ul = u0 + dy0 * slope_u1;
		float ur = u1 + dy1 * slope_u2;
		float vl = v0 + dy0 * slope_v1;
		float vr = v1 + dy1 * slope_v2;

		float xnl = xn0 + dy0 * slope_xn1;
		float xnr = xn1 + dy1 * slope_xn2;
		float ynl = yn0 + dy0 * slope_yn1;
		float ynr = yn1 + dy1 * slope_yn2;
		float znl = zn0 + dy0 * slope_zn1;
		float znr = zn1 + dy1 * slope_zn2;

		float invWl = invW0 + dy0 * slope_invW1;
		float invWr = invW1 + dy1 * slope_invW2;

		if (xl > xr)
		{
			std::swap(xl, xr);
			std::swap(zl, zr);
			std::swap(ul, ur);
			std::swap(vl, vr);
			std::swap(xnl, xnr);
			std::swap(ynl, ynr);
			std::swap(znl, znr);
			std::swap(invWl, invWr);
		}

		DrawHorizontalDepthTextureLine(xl, xr, y, zl, zr, ul, ur, vl, vr, xnl, xnr, ynl, ynr, znl, znr, invWl, invWr, texture);
	}
}

void Renderer::DrawHorizontalDepthTextureLine(
	float x0, float x1, int y,
	float z0, float z1,
	float u0, float u1,
	float v0, float v1,
	float xn0, float xn1,
	float yn0, float yn1,
	float zn0, float zn1,
	float invW0, float invW1,
	const Texture& texture
)
{
	if (y < 0 || y >= m_Height) return;

	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(z0, z1);
		std::swap(u0, u1);
		std::swap(v0, v1);
		std::swap(xn0, xn1);
		std::swap(yn0, yn1);
		std::swap(zn0, zn1);
		std::swap(invW0, invW1);
	}

	if (x0 < -0.5f)
	{
		float t = (-0.5f - x0) / (float)(x1 - x0);
		z0 += t * (z1 - z0);
		u0 += t * (u1 - u0);
		v0 += t * (v1 - v0);
		xn0 += t * (xn1 - xn0);
		yn0 += t * (yn1 - yn0);
		zn0 += t * (zn1 - zn0);
		invW0 += t * (invW1 - invW0);
		x0 = -0.5f;
	}
	if (x1 >= m_Width - 0.5f)
	{
		float t = (m_Width - 0.5f - x0) / (float)(x1 - x0);
		z1 = z0 + t * (z1 - z0);
		u1 = u0 + t * (u1 - u0);
		v1 = v0 + t * (v1 - v0);
		xn1 = xn0 + t * (xn1 - xn0);
		yn1 = yn0 + t * (yn1 - yn0);
		zn1 = zn0 + t * (zn1 - zn0);
		invW1 = invW0 + t * (invW1 - invW0);
		x1 = m_Width - 0.5f;
	}

	int x_start = (int)ceilf(x0 - 0.5f);
	int x_end = (int)ceilf(x1 - 0.5f);

	float dx = x1 - x0;
	if (dx <= 1e-3f)
	{
		int x = (int)floorf((x0 + x1) * 0.5f);
		float z = (z0 + z1) / 2;
		float u = (u0 + u1) / 2;
		float v = (v0 + v1) / 2;

		float invW = (invW0 + invW1) / 2;

		float xn = (xn0 + xn1) / (invW * 2);
		float yn = (yn0 + yn1) / (invW * 2);
		float zn = (zn0 + zn1) / (invW * 2);

		if (0)
		{
			DrawDepthPixel(x, y, z, texture.GetPixelAtUV(u / invW, v / invW));
		}
		else
		{
			Vec3f normal = Vec3f(xn, yn, zn).normalized();
			uint32_t textureColor = texture.GetPixelAtUV(u / invW, v / invW);
			DrawDepthPixel(x, y, z, ComputeShadedColorForNormal(normal, m_DirectionalLight, m_AmbientLight, textureColor));
		}
		return;
	}

	float dz = (z1 - z0) / dx;
	float du = (u1 - u0) / dx;
	float dv = (v1 - v0) / dx;
	float dxn = (xn1 - xn0) / dx;
	float dyn = (yn1 - yn0) / dx;
	float dzn = (zn1 - zn0) / dx;
	
	float dinvW = (invW1 - invW0) / dx;

	float t2 = (x_start + 0.5f - x0);

	float z = z0 + t2 * dz;
	float u = u0 + t2 * du;
	float v = v0 + t2 * dv;
	float xn = xn0 + t2 * dxn;
	float yn = yn0 + t2 * dyn;
	float zn = zn0 + t2 * dzn;

	float invW = invW0 + t2 * dinvW;

	for (int x = x_start; x < x_end; x++)
	{
		if (0)
		{
			DrawDepthPixel(x, y, z, texture.GetPixelAtUV(u / invW, v / invW));
		}
		else
		{
			Vec3f normal = Vec3f(xn, yn, zn).normalized();
			uint32_t textureColor = texture.GetPixelAtUV(u / invW, v / invW);
			DrawDepthPixel(x, y, z, ComputeShadedColorForNormal(normal, m_DirectionalLight, m_AmbientLight, textureColor));
		}

		z += dz;
		u += du;
		v += dv;
		xn += dxn;
		yn += dyn;
		zn += dzn;

		invW += dinvW;
	}
}
