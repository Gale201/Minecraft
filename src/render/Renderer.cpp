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

				float u0w = triangle.v0.u * triangle.v0.invW;
				float u1w = triangle.v1.u * triangle.v1.invW;
				float u2w = triangle.v2.u * triangle.v2.invW;

				float v0w = triangle.v0.v * triangle.v0.invW;
				float v1w = triangle.v1.v * triangle.v1.invW;
				float v2w = triangle.v2.v * triangle.v2.invW;

				float invW = w0 * triangle.v0.invW + w1 * triangle.v1.invW + w2 * triangle.v2.invW;
				float u = (w0 * u0w + w1 * u1w + w2 * u2w) / invW;
				float v = (w0 * v0w + w1 * v1w + w2 * v2w) / invW;

				DrawDepthPixel(x, y, z, texture.GetPixelAtUV(u, v));
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

void Renderer::DrawTriangleScanline(const Triangle& triangle, const Texture& texture)
{
	Triangle sorted = SortTriangleVerticesByY(triangle);

	Vertex v0 = sorted.v0;
	Vertex v1 = sorted.v1;
	Vertex v2 = sorted.v2;

	v0.u = v0.u * v0.invW;
	v1.u = v1.u * v1.invW;
	v2.u = v2.u * v2.invW;
	v0.v = v0.v * v0.invW;
	v1.v = v1.v * v1.invW;
	v2.v = v1.v * v2.invW;

	if ((int)v1.position.y == (int)v2.position.y)
	{
		DrawFlatBottomTriangle(
			v0.position.x, v0.position.y, v0.position.z, v0.u, v0.v, 0, 0, 0,
			v1.position.x, v1.position.y, v1.position.z, v1.u, v1.v, 0, 0, 0,
			v2.position.x, v2.position.y, v2.position.z, v2.u, v2.v, 0, 0, 0,
			texture
		);
	}
	else if ((int)v0.position.y == (int)v1.position.y)
	{
		DrawFlatTopTriangle(
			v0.position.x, v0.position.y, v0.position.z, v0.u, v0.v, 0, 0, 0,
			v1.position.x, v1.position.y, v1.position.z, v1.u, v1.v, 0, 0, 0,
			v2.position.x, v2.position.y, v2.position.z, v2.u, v2.v, 0, 0, 0,
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

		DrawFlatBottomTriangle(
			v0.position.x, v0.position.y, v0.position.z, v0.u, v0.v, 0, 0, 0,
			v1.position.x, v1.position.y, v1.position.z, v1.u, v1.v, 0, 0, 0,
			vNew.position.x, vNew.position.y, vNew.position.z, vNew.u, vNew.v, 0, 0, 0,
			texture
		);
		DrawFlatTopTriangle(
			v1.position.x, v1.position.y, v1.position.z, v1.u, v1.v, 0, 0, 0,
			vNew.position.x, vNew.position.y, vNew.position.z, vNew.u, vNew.v, 0, 0, 0,
			v2.position.x, v2.position.y, v2.position.z, v2.u, v2.v, 0, 0, 0,
			texture
		);
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

					Vertex screenVertex{ {}, clippedVertex.normal, clippedVertex.u, clippedVertex.v };

					// Convert to screen space
					screenVertex.position.x = (perspectiveVertex.x * 0.5f + 0.5f) * m_Width;
					screenVertex.position.y = (1.0f - (perspectiveVertex.y * 0.5f + 0.5f)) * m_Height;
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


void Renderer::DrawFlatBottomTriangle(
	float x0, float y0, float z0, float u0, float v0, float xn0, float yn0, float zn0,
	float x1, float y1, float z1, float u1, float v1, float xn1, float yn1, float zn1,
	float x2, float y2, float z2, float u2, float v2, float xn2, float yn2, float zn2,
	Texture texture
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

		if (xl > xr)
		{
			std::swap(xl, xr);
			std::swap(zl, zr);
			std::swap(ul, ur);
			std::swap(vl, vr);
			std::swap(xnl, xnr);
			std::swap(ynl, ynr);
			std::swap(znl, znr);
		}

		DrawHorizontalDepthTextureLine(xl, xr, y, zl, zr, ul, ur, vl, vr, xnl, xnr, ynl, ynr, znl, znr, texture);
	}
}

void Renderer::DrawFlatTopTriangle(
	float x0, float y0, float z0, float u0, float v0, float xn0, float yn0, float zn0,
	float x1, float y1, float z1, float u1, float v1, float xn1, float yn1, float zn1,
	float x2, float y2, float z2, float u2, float v2, float xn2, float yn2, float zn2,
	Texture texture
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

		if (xl > xr)
		{
			std::swap(xl, xr);
			std::swap(zl, zr);
			std::swap(ul, ur);
			std::swap(vl, vr);
			std::swap(xnl, xnr);
			std::swap(ynl, ynr);
			std::swap(znl, znr);
		}

		DrawHorizontalDepthTextureLine(xl, xr, y, zl, zr, ul, ur, vl, vr, xnl, xnr, ynl, ynr, znl, znr, texture);
	}
}

void Renderer::DrawHorizontalDepthTextureLine(
	float x0, float x1,
	int y,
	float inv_z0, float inv_z1,
	float u0_over_z, float u1_over_z,
	float v0_over_z, float v1_over_z,
	float xn0_over_z, float xn1_over_z,
	float yn0_over_z, float yn1_over_z,
	float zn0_over_z, float zn1_over_z,
	Texture texture
)
{
	if (y < 0 || y >= m_Height) return;

	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(inv_z0, inv_z1);
		std::swap(u0_over_z, u1_over_z);
		std::swap(v0_over_z, v1_over_z);
		std::swap(xn0_over_z, xn1_over_z);
		std::swap(yn0_over_z, yn1_over_z);
		std::swap(zn0_over_z, zn1_over_z);
	}

	float xl = x0;
	float xr = x1;
	float inv_zl = inv_z0;
	float inv_zr = inv_z1;
	float uoz_l = u0_over_z;
	float uoz_r = u1_over_z;
	float voz_l = v0_over_z;
	float voz_r = v1_over_z;
	float xnoz_l = xn0_over_z;
	float xnoz_r = xn1_over_z;
	float ynoz_l = yn0_over_z;
	float ynoz_r = yn1_over_z;
	float znoz_l = zn0_over_z;
	float znoz_r = zn1_over_z;

	if (xl < -0.5f)
	{
		float t = (-0.5f - xl) / (float)(xr - xl);
		inv_zl += t * (inv_zr - inv_zl);
		uoz_l += t * (uoz_r - uoz_l);
		voz_l += t * (voz_r - voz_l);
		xnoz_l += t * (xnoz_r - xnoz_l);
		ynoz_l += t * (ynoz_r - ynoz_l);
		znoz_l += t * (znoz_r - znoz_l);
		xl = -0.5f;
	}
	if (xr >= m_Width - 0.5f)
	{
		float t = (m_Width - 0.5f - xl) / (float)(xr - xl);
		inv_zr = inv_zl + t * (inv_zr - inv_zl);
		uoz_r = uoz_l + t * (uoz_r - uoz_l);
		voz_r = voz_l + t * (voz_r - voz_l);
		xnoz_r = xnoz_l + t * (xnoz_r - xnoz_l);
		ynoz_r = ynoz_l + t * (ynoz_r - ynoz_l);
		znoz_r = znoz_l + t * (znoz_r - znoz_l);
		xr = m_Width - 0.5f;
	}

	int x_start = (int)ceilf(xl - 0.5f);
	int x_end = (int)ceilf(xr - 0.5f);

	float dx = xr - xl;
	if (dx <= 1e-3f)
	{
		int x = (int)floorf((xl + xr) * 0.5f);
		float inv_z = (inv_zl + inv_zr) / 2;
		float uoz = (uoz_l + uoz_r) / 2;
		float voz = (voz_l + voz_r) / 2;
		float xnoz = (xnoz_l + xnoz_r) / (inv_z * 2);
		float ynoz = (ynoz_l + ynoz_r) / (inv_z * 2);
		float znoz = (znoz_l + znoz_r) / (inv_z * 2);

		DrawDepthPixel(x, y, inv_z, texture.GetPixelAtUV(uoz / inv_z, voz / inv_z));
		return;
	}

	float dinv_z = (inv_zr - inv_zl) / dx;
	float duoz = (uoz_r - uoz_l) / dx;
	float dvoz = (voz_r - voz_l) / dx;
	float dxnoz = (xnoz_r - xnoz_l) / dx;
	float dynoz = (ynoz_r - ynoz_l) / dx;
	float dznoz = (znoz_r - znoz_l) / dx;

	float t2 = (x_start + 0.5f - xl);

	float inv_z = inv_zl + t2 * dinv_z;
	float uoz = uoz_l + t2 * duoz;
	float voz = voz_l + t2 * dvoz;
	float xnoz = xnoz_l + t2 * dxnoz;
	float ynoz = ynoz_l + t2 * dynoz;
	float znoz = znoz_l + t2 * dznoz;

	for (int x = x_start; x < x_end; x++)
	{

		DrawDepthPixel(x, y, inv_z, texture.GetPixelAtUV(uoz / inv_z, voz / inv_z));

		inv_z += dinv_z;
		uoz += duoz;
		voz += dvoz;
		xnoz += dxnoz;
		ynoz += dynoz;
		znoz += dznoz;
	}
}
