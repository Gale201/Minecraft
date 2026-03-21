#include "Renderer.h"

#include <limits>
#include <algorithm>
#include <assert.h>

#include "RenderCommand.h"
#include "../core/Log.h"
#include "../timer/Timer.h"
#include "../timer/AccumTimer.h"
#include "../world/BlockFace.h"

namespace Minecraft
{

// Destructor & init

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

// Primitive methods

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

inline void Renderer::DrawPixelAlphaBlend(int x, int y, uint32_t color)
{
	//if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) return;

	int pixel = y * m_Width + x;
	uint32_t dest = m_FrameBuffer[pixel];

	uint8_t srcA = (color >> 24) & 0xFF;
	if (srcA == 0) return;

	uint32_t invA = 255 - srcA;

	uint8_t srcR = (color >> 16) & 0xFF;
	uint8_t srcG = (color >> 8) & 0xFF;
	uint8_t srcB = color & 0xFF;

	uint8_t destR = (dest >> 16) & 0xFF;
	uint8_t destG = (dest >> 8) & 0xFF;
	uint8_t destB = dest & 0xFF;

	uint8_t outR = (srcR * srcA + destR * invA) >> 8;
	uint8_t outG = (srcG * srcA + destG * invA) >> 8;
	uint8_t outB = (srcB * srcA + destB * invA) >> 8;

	uint32_t outColor = (0xFF << 24) | (outR << 16) | (outG << 8) | outB;

	m_FrameBuffer[pixel] = outColor;

	/*

	uint8_t srcA = (color >> 24) & 0xFF;
	uint8_t srcR = (color >> 16) & 0xFF;
	uint8_t srcG = (color >> 8) & 0xFF;
	uint8_t srcB = color & 0xFF;

	float alpha = srcA / 255.0f;

	uint8_t destR = (dest >> 16) & 0xFF;
	uint8_t destG = (dest >> 8) & 0xFF;
	uint8_t destB = dest & 0xFF;

	uint8_t outR = static_cast<uint8_t>(srcR * alpha + destR * (1.0f - alpha));
	uint8_t outG = static_cast<uint8_t>(srcG * alpha + destG * (1.0f - alpha));
	uint8_t outB = static_cast<uint8_t>(srcB * alpha + destB * (1.0f - alpha));

	uint32_t outColor = (0xFF << 24) | (outR << 16) | (outG << 8) | outB;

	m_FrameBuffer[pixel] = outColor;
	*/
}

inline void Renderer::DrawDepthPixelAlphaBlend(int x, int y, float z, uint32_t color, bool writeDepth)
{
	if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) return;

	if (z < m_DepthBuffer[y * m_Width + x])
	{
		if (writeDepth) m_DepthBuffer[y * m_Width + x] = z;
		DrawPixelAlphaBlend(x, y, color);
	}
}

// Drawing lines, triangles and objects

void Renderer::DrawLine(Vec3f start, Vec3f end, uint32_t color, int width, bool alphaBlend)
{
	float dx = end.x - start.x;
	float dy = end.y - start.y;
	float dz = end.z - start.z;

	int steps = static_cast<int>(std::max(std::abs(dx), std::abs(dy)));
	if (steps == 0)
		return;

	float xStep = dx / steps;
	float yStep = dy / steps;
	float zStep = dz / steps;

	float x = start.x;
	float y = start.y;
	float z = start.z;

	int widthNeg = -static_cast<int>(std::floorf((float)width / 2.0f));
	int widthPos = static_cast<int>(std::ceilf((float)width / 2.0f));
	for (int i = 0; i <= steps; i++)
	{
		for (int wy = widthNeg; wy <= widthPos; wy++)
		{
			for (int wx = widthNeg; wx <= widthPos; wx++)
			{
				if (alphaBlend)
					DrawDepthPixelAlphaBlend(static_cast<int>(x + wx), static_cast<int>(y + wy), z, color);
				else
					DrawDepthPixel(static_cast<int>(x + wx), static_cast<int>(y + wy), z, color);
			}
		}
		x += xStep;
		y += yStep;
		z += zStep;
	}
}

void Renderer::DrawTriangleBarycentric(const Triangle& triangle, const Texture& texture, AtlasTileInfo tileInfo)
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
			float px = x + 0.5f;
			float py = y + 0.5f;

			// Barycentric coordinates
			float w0 = (
				(triangle.v1.position.y - triangle.v2.position.y) * (px - triangle.v2.position.x) +
				(triangle.v2.position.x - triangle.v1.position.x) * (py - triangle.v2.position.y)
				) / divisor;

			float w1 = (
				(triangle.v2.position.y - triangle.v0.position.y) * (px - triangle.v2.position.x) +
				(triangle.v0.position.x - triangle.v2.position.x) * (py - triangle.v2.position.y)
				) / divisor;

			float w2 = 1.0f - w0 - w1;

			// If the pixel is inside the triangle
			float eps = 0.0f;
			if (w0 >= eps && w1 >= eps && w2 >= eps)
			{
				float z = w0 * triangle.v0.position.z + w1 * triangle.v1.position.z + w2 * triangle.v2.position.z;

				// Early depth rejection
				if (z >= m_DepthBuffer[y * m_Width + x])
					continue;

				float invW = w0 * triangle.v0.invW + w1 * triangle.v1.invW + w2 * triangle.v2.invW;
				float u = (w0 * triangle.v0.u + w1 * triangle.v1.u + w2 * triangle.v2.u) / invW;
				float v = (w0 * triangle.v0.v + w1 * triangle.v1.v + w2 * triangle.v2.v) / invW;

				if (m_PerPixelLighting)
				{
					Vec3f normal = (triangle.v0.normal * w0 + triangle.v1.normal * w1 + triangle.v2.normal * w2).normalized();
					uint32_t textureColor = texture.GetPixelAtUV(u, v);
					DrawDepthPixel(x, y, z, ComputeShadedColorForNormal(normal, m_DirectionalLight, m_AmbientLight, textureColor));
				}
				else
				{
					u -= (int)u;
					v -= (int)v;

					constexpr float inset = 0.01f / 160.f;

					float atlasU = tileInfo.u0 + inset + u * (tileInfo.uScale - 2.0f * inset);
					float atlasV = tileInfo.v0 + inset + v * (tileInfo.vScale - 2.0f * inset);

					DrawDepthPixel(x, y, z, ShadeColor(texture.GetPixelAtUV(atlasU, atlasV), triangle.lightIntensity));
				}
			}
		}
	}
}

void Renderer::DrawTriangleScanline(const Triangle& triangle, const Texture& texture, AtlasTileInfo tileInfo)
{
	Triangle sorted = SortTriangleVerticesByY(triangle);

	Vertex v0 = sorted.v0;
	Vertex v1 = sorted.v1;
	Vertex v2 = sorted.v2;

	if ((int)std::ceilf(v1.position.y - 0.5f) == (int)std::ceilf(v2.position.y - 0.5f))
	{
		DrawFlatBottomTriangle(
			v0.position.x, v0.position.y, v0.position.z, v0.u, v0.v, v0.normal.x, v0.normal.y, v0.normal.z, v0.invW,
			v1.position.x, v1.position.y, v1.position.z, v1.u, v1.v, v1.normal.x, v1.normal.y, v1.normal.z, v1.invW,
			v2.position.x, v2.position.y, v2.position.z, v2.u, v2.v, v2.normal.x, v2.normal.y, v2.normal.z, v2.invW,
			texture, triangle.lightIntensity, tileInfo
		);
	}
	else if ((int)std::ceilf(v0.position.y - 0.5f) == (int)std::ceilf(v1.position.y - 0.5f))
	{
		DrawFlatTopTriangle(
			v0.position.x, v0.position.y, v0.position.z, v0.u, v0.v, v0.normal.x, v0.normal.y, v0.normal.z, v0.invW,
			v1.position.x, v1.position.y, v1.position.z, v1.u, v1.v, v1.normal.x, v1.normal.y, v1.normal.z, v1.invW,
			v2.position.x, v2.position.y, v2.position.z, v2.u, v2.v, v2.normal.x, v2.normal.y, v2.normal.z, v2.invW,
			texture, triangle.lightIntensity, tileInfo
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
			texture, triangle.lightIntensity, tileInfo
		);
		DrawFlatTopTriangle(
			v1.position.x, v1.position.y, v1.position.z, v1.u, v1.v, v1.normal.x, v1.normal.y, v1.normal.z, v1.invW,
			vNew.position.x, vNew.position.y, vNew.position.z, vNew.u, vNew.v, vNew.normal.x, vNew.normal.y, vNew.normal.z, vNew.invW,
			v2.position.x, v2.position.y, v2.position.z, v2.u, v2.v, v2.normal.x, v2.normal.y, v2.normal.z, v2.invW,
			texture, triangle.lightIntensity, tileInfo
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
			float edgeThreshold = 1.0f / (float)std::max(right - left, bottom - top);
			if (w0 >= 0 && w1 >= 0 && w2 >= 0 && (w0 < edgeThreshold || w1 < edgeThreshold || w2 < edgeThreshold))
			{
				float z = w0 * triangle.v0.position.z + w1 * triangle.v1.position.z + w2 * triangle.v2.position.z;
				DrawDepthPixel(x, y, z - 0.001f, color);
			}
		}
	}
}

void Renderer::DrawCubeWireframe(const Transform& transform, uint32_t color, const Camera& camera, int width, bool alphaBlend, bool centered)
{
	Mat4 viewMatrix = camera.GetViewMatrix();
	Mat4 perspectiveMatrix = camera.GetPerspectiveMatrix();

	Vec3f vertices[8];
	if (centered)
	{
		vertices[0] = Vec3f(-0.5f, -0.5f, -0.5f);
		vertices[1] = Vec3f(0.5f, -0.5f, -0.5f);
		vertices[2] = Vec3f(0.5f, -0.5f, 0.5f);
		vertices[3] = Vec3f(-0.5f, -0.5f, 0.5f);
		vertices[4] = Vec3f(-0.5f, 0.5f, -0.5f);
		vertices[5] = Vec3f(0.5f, 0.5f, -0.5f);
		vertices[6] = Vec3f(0.5f, 0.5f, 0.5f);
		vertices[7] = Vec3f(-0.5f, 0.5f, 0.5f);
	}
	else
	{
		vertices[0] = Vec3f(0.0f, 0.0f, 0.0f);
		vertices[1] = Vec3f(1.0f, 0.0f, 0.0f);
		vertices[2] = Vec3f(1.0f, 0.0f, 1.0f);
		vertices[3] = Vec3f(0.f, 0.f, 1.f);
		vertices[4] = Vec3f(0.f, 1.f, 0.f);
		vertices[5] = Vec3f(1.f, 1.f, 0.f);
		vertices[6] = Vec3f(1.f, 1.f, 1.f);
		vertices[7] = Vec3f(0.f, 1.f, 1.f);
	}

	Vec3f projected[8];
	bool clipped[8]{};
	for (int i = 0; i < 8; i++)
	{
		Vec3f vertex = vertices[i];
		Vec4f worldVertex = transform.GetMatrix() * Vec4f(vertex.x, vertex.y, vertex.z, 1.0f);
		Vec4f cameraVertex = viewMatrix * worldVertex;
		Vec4f perspectiveVertex = perspectiveMatrix * cameraVertex;

		perspectiveVertex.x /= perspectiveVertex.w;
		perspectiveVertex.y /= perspectiveVertex.w;
		perspectiveVertex.z /= perspectiveVertex.w;

		Vec3f screenVertex;
		screenVertex.x = (perspectiveVertex.x * 0.5f + 0.5f) * m_Width;
		screenVertex.y = (1.0f - (perspectiveVertex.y * 0.5f + 0.5f)) * m_Height;
		screenVertex.z = perspectiveVertex.z;

		if (perspectiveVertex.w <= 0.0f)
			clipped[i] = true;
		
		projected[i] = Vec3f(screenVertex.x, screenVertex.y, screenVertex.z);
	}

	for (int i = 0; i < 4; i++)
	{
		if (!clipped[i] && !clipped[(i + 1) % 4]) 
			DrawLine(projected[i], projected[(i + 1) % 4], color, width, alphaBlend);
	}

	for (int i = 4; i < 8; i++)
	{
		if (!clipped[i] && !clipped[4 + (i + 1) % 4])
			DrawLine(projected[i], projected[4 + (i + 1) % 4], color, width, alphaBlend);
	}

	for (int i = 0; i < 4; i++)
	{
		if (!clipped[i] && !clipped[i + 4])
			DrawLine(projected[i], projected[i + 4], color, width, alphaBlend);
	}
}

void Renderer::DrawMesh(
	const Mesh& mesh,
	const Transform& transform,
	const Texture& texture,
	const Camera& camera
)
{
	Mat4 modelMatrix = transform.GetMatrix();
	Mat4 viewMatrix = camera.GetViewMatrix();
	Mat4 perspectiveMatrix = camera.GetPerspectiveMatrix();

	Vec3f transformedDirectionalLight = (
		viewMatrix * Vec4f::FromVec3f(m_DirectionalLight, 0.0f)
		).ToVec3f(); // Add .normalized() for a classic directional light
	
	Triangle triangle;
	AtlasTileInfo tileInfo;
	for (size_t i = 0; i < mesh.indices.size(); i += 3)
	{
		for (size_t j = 0; j < 3; j++)
		{
			// Get the model vertex from the mesh
			const Vertex& v = mesh.vertices[mesh.indices[i + j]];

			tileInfo = v.tileInfo;

			// Transform vertex from model space to world space
			Vec4f worldVertex = modelMatrix * Vec4f(v.position.x, v.position.y, v.position.z, 1.0f);

			// Transform world vertex to camera space
			Vec4f cameraVertex = viewMatrix * worldVertex;

			triangle.SetVertexByIndex(j, { cameraVertex.ToVec3f(), v.normal, v.u, v.v, 0.0f, v.tileInfo });	
		}

		// Back face culling (happening in camera space)
		if (IsTriangleBackFacing(triangle))
			continue;

		// Near plane clipping (happening in camera space)
		Triangle clipped[2];
		int nClipped = ClipTriangleNearPlane(triangle, clipped, camera.nearPlane);

		// Compute light intensity for the triangle
		triangle.lightIntensity = ComputeLightIntensityForTriangle(
			triangle,
			transformedDirectionalLight,
			m_AmbientLight
		);

		for (size_t c = 0; c < nClipped; c++)
		{
			// Loop over vertices and project them
			for (int j = 0; j < 3; j++)
			{
				// Clipped vertex
				Vertex clippedVertex = clipped[c].GetVertexByIndex(j);

				// Transform the clipped vertex to clip space
				Vec4f perspectiveVertex = perspectiveMatrix * Vec4f::FromVec3f(clippedVertex.position, 1.0f);

				// Perspective divide
				perspectiveVertex.x /= perspectiveVertex.w;
				perspectiveVertex.y /= perspectiveVertex.w;
				perspectiveVertex.z /= perspectiveVertex.w;

				clippedVertex.u /= perspectiveVertex.w;
				clippedVertex.v /= perspectiveVertex.w;

				Vertex screenVertex{ {}, clippedVertex.normal, clippedVertex.u, clippedVertex.v };

				// Convert to screen space
				screenVertex.position.x = (perspectiveVertex.x * 0.5f + 0.5f) * m_Width;
				screenVertex.position.y = (1.0f - (perspectiveVertex.y * 0.5f + 0.5f)) * m_Height;
				screenVertex.position.z = perspectiveVertex.z;
				screenVertex.invW = 1.0f / perspectiveVertex.w;

				triangle.SetVertexByIndex(j, screenVertex);
			}

			// Screen space back face culling
			if (CalculateTriangleArea(triangle) <= 0) continue;

			DrawTriangleBarycentric(triangle, texture, tileInfo);
		}
	}
}

void Renderer::DrawRenderObject(const RenderObject& renderObject, const Camera& camera)
{
	Mat4 modelMatrix = renderObject.transform.GetMatrix();
	Mat4 viewMatrix = camera.GetViewMatrix();
	Mat4 perspectiveMatrix = camera.GetPerspectiveMatrix();

	Vec3f transformedDirectionalLight = (
		viewMatrix * Vec4f::FromVec3f(m_DirectionalLight, 0.0f)
	).ToVec3f(); // Add .normalized() for a classic directional light

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
				Vec4f worldVertex = modelMatrix * Vec4f(v.position.x, v.position.y, v.position.z, 1.0f);

				// Transform world vertex to camera space
				Vec4f cameraVertex = viewMatrix * worldVertex;

				triangle.SetVertexByIndex(j, { cameraVertex.ToVec3f(), v.normal, v.u, v.v });
			}

			// Back face culling (happening in camera space)
			if (IsTriangleBackFacing(triangle))
				continue;

			// Near plane clipping (happening in camera space)
			Triangle clipped[2];
			int nClipped = ClipTriangleNearPlane(triangle, clipped, camera.nearPlane);

			// Compute light intensity for the triangle
			triangle.lightIntensity = ComputeLightIntensityForTriangle(
				triangle,
				transformedDirectionalLight,
				m_AmbientLight
			);

			for (size_t c = 0; c < nClipped; c++)
			{
				// Loop over vertices and project them
				for (int j = 0; j < 3; j++)
				{
					// Clipped vertex
					Vertex clippedVertex = clipped[c].GetVertexByIndex(j);

					// Transform the clipped vertex to clip space
					Vec4f perspectiveVertex = perspectiveMatrix * Vec4f::FromVec3f(clippedVertex.position, 1.0f);

					// Perspective divide
					perspectiveVertex.x /= perspectiveVertex.w;
					perspectiveVertex.y /= perspectiveVertex.w;
					perspectiveVertex.z /= perspectiveVertex.w;

					clippedVertex.u /= perspectiveVertex.w;
					clippedVertex.v /= perspectiveVertex.w;

					Vertex screenVertex{ {}, clippedVertex.normal, clippedVertex.u, clippedVertex.v };

					// Convert to screen space
					screenVertex.position.x = static_cast<float>((perspectiveVertex.x * 0.5f + 0.5f) * m_Width);
					screenVertex.position.y = static_cast<float>((1.0f - (perspectiveVertex.y * 0.5f + 0.5f)) * m_Height);
					screenVertex.position.z = perspectiveVertex.z;
					screenVertex.invW = 1.0f / perspectiveVertex.w;

					triangle.SetVertexByIndex(j, screenVertex);
				}

				// Screen space back face culling
				if (CalculateTriangleArea(triangle) <= 0) continue;

				DrawTriangleBarycentric(triangle, renderObject.texture, {});
			}
		}
	}
}

void Renderer::DrawTexture(int x, int y, const Texture& texture, bool alphaBlend)
{
	for (int i = 0; i < texture.GetHeight(); i++)
	{
		for (int j = 0; j < texture.GetWidth(); j++)
		{
			uint32_t color = texture.GetPixel(i, j);

			if (alphaBlend)
				DrawPixelAlphaBlend(x + j, y + i, color);
			else
				DrawPixel(x + j, y + i, color);
		}
	}
}

void Renderer::DrawTexture(int x, int y, const Texture& texture, int width, int height, bool alphaBlend, bool cutoff, uint32_t maskColor)
{
	if (width <= 0 || height <= 0) return;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			float u = (float)j / (float)width;
			float v = (float)i / (float)height;

			uint32_t color = texture.GetPixelAtUV(u, v);

			if (alphaBlend)
				DrawPixelAlphaBlend(x + j, y + i, color);
			else if (cutoff)
			{
				uint8_t alpha = (color >> 24) & 0xFF;
				if (maskColor != 0) color = maskColor;
				if (alpha > 128)
					DrawPixel(x + j, y + i, color);
			}
			else
				DrawPixel(x + j, y + i, color);
		}
	}
}

void Renderer::DrawText(const std::string& text, int x, int y, int width, int height, const FontLoader& fontLoader, bool shadow)
{
	int cursorX = x;
	int cursorY = y;
	for (char c : text)
	{
		if (c == '\n')
		{
			cursorX = x;
			cursorY += height;
			continue;
		}
		const Texture* charTexture = fontLoader.GetCharacterTexture(c);
		if (charTexture)
		{
			if (shadow)
				DrawTexture(
					cursorX + static_cast<int>(width / (float)fontLoader.GetCharWidth()),
					cursorY + static_cast<int>(height / (float)fontLoader.GetCharHeight()),
					*charTexture, width, height, false, true, 0xFF444444
					);
			DrawTexture(cursorX, cursorY, *charTexture, width, height, false, true);
			cursorX += width;
		}
	}
}

void Renderer::Render(const std::vector<RenderCommand>& renderList, const Camera& camera, Core::ThreadSystem& threadSystem)
{
	m_ActiveRenderList = &renderList;
	m_ActiveCamera = &camera;

	{
		Core::Timer timer("Build screen triangles");

		int threadCount = (int)threadSystem.GetThreadCount();
		std::vector<std::vector<BoxedScreenTriangle>> threadBuffers(threadCount);

		threadSystem.ParallelFor(
			(int)m_ActiveRenderList->size(),
			[&](int index)
			{
				int threadIndex = (int)threadSystem.GetCurrentThreadIndex();
				auto& threadBuffer = threadBuffers[threadIndex];

				const RenderCommand& command = m_ActiveRenderList->at(index);
				
				BuildScreenTriangles(
					*command.mesh,
					command.transform,
					command.texture,
					*m_ActiveCamera,
					threadBuffer
				);
			}
		);

		threadSystem.ParallelFor(
			(int)m_ScreenTiles.size(),
			[&](int tileIndex)
			{
				ScreenTile& tile = m_ScreenTiles[tileIndex];
				tile.opaqueTriangles.clear();
				tile.transparentTriangles.clear();

				int tilesPerRow = (m_Width + TILE_SIZE - 1) / TILE_SIZE;
				int tilesPerCol = (m_Height + TILE_SIZE - 1) / TILE_SIZE;

				int tx = tileIndex % tilesPerRow;
				int ty = tileIndex / tilesPerRow;

				for (auto& threadBuffer : threadBuffers)
				{
					for (auto& buffer : threadBuffer)
					{
						if (tx >= buffer.tileLeft && tx <= buffer.tileRight && 
							ty >= buffer.tileTop && ty <= buffer.tileBottom)
						{
							if (buffer.triangle.tileInfo.isTransparent)
								tile.transparentTriangles.emplace_back(std::move(buffer.triangle));
							else
								tile.opaqueTriangles.emplace_back(std::move(buffer.triangle));
						}
					}
				}
			}
		);
	}

	Core::Timer timer("Rasterize tiles");
	threadSystem.ParallelFor(
		(int)m_ScreenTiles.size(),
		[this](int tileIndex)
		{
			RasterizeTile(m_ScreenTiles[tileIndex]);
		}
	);
}

void Renderer::CreateTiles()
{
	m_ScreenTiles.clear();
	for (int y = 0; y < m_Height; y += TILE_SIZE)
	{
		for (int x = 0; x < m_Width; x += TILE_SIZE)
		{
			ScreenTile tile;
			tile.x0 = x;
			tile.y0 = y;
			tile.x1 = std::min(x + TILE_SIZE - 1, m_Width - 1);
			tile.y1 = std::min(y + TILE_SIZE - 1, m_Height - 1);
			tile.opaqueTriangles.reserve(512);
			tile.transparentTriangles.reserve(256);
			m_ScreenTiles.push_back(tile);
		}
	}
}

// Private functions

void Renderer::RasterizeTile(ScreenTile& tile)
{
	for (const ScreenTriangle& triangle : tile.opaqueTriangles)
	{
		DrawTriangleBarycentricToTile(triangle.triangle, *triangle.texture, triangle.tileInfo, tile);
		//DrawTriangleWireframeToTile(triangle.triangle, tile, 0);
	}

	std::sort(tile.transparentTriangles.begin(), tile.transparentTriangles.end(), 
		[&](const ScreenTriangle& a, const ScreenTriangle& b)
		{
			return a.distToCamera > b.distToCamera;
		});

	for (const ScreenTriangle& triangle : tile.transparentTriangles)
	{
		DrawTriangleBarycentricToTile(triangle.triangle, *(triangle.texture), triangle.tileInfo, tile);
		//DrawTriangleWireframeToTile(triangle.triangle, tile, 0);
	}
}

void Renderer::BuildScreenTriangles(
	const Mesh& mesh,
	const Transform& transform,
	const Texture& texture,
	const Camera& camera,
	std::vector<BoxedScreenTriangle>& out
)
{
	Mat4 modelMatrix = transform.GetMatrix();
	Mat4 viewMatrix = camera.GetViewMatrix();
	Mat4 perspectiveMatrix = camera.GetPerspectiveMatrix();

	Vec3f transformedDirectionalLight = (viewMatrix * Vec4f::FromVec3f(m_DirectionalLight, 0.0f)).ToVec3f(); // Add .normalized() for a classic directional light

	Triangle triangle;
	AtlasTileInfo tileInfo;
	float distToCamera = 0.0f;
	for (size_t i = 0; i < mesh.indices.size(); i += 3)
	{
		for (size_t j = 0; j < 3; j++)
		{
			// Get the model vertex from the mesh
			const Vertex& v = mesh.vertices[mesh.indices[i + j]];

			tileInfo = v.tileInfo;

			// Transform vertex from model space to world space
			Vec4f worldVertex = modelMatrix * Vec4f(v.position.x, v.position.y, v.position.z, 1.0f);

			// Transform world vertex to camera space
			Vec4f cameraVertex = viewMatrix * worldVertex;

			triangle.SetVertexByIndex(j, { cameraVertex.ToVec3f(), v.normal, v.u, v.v, 0.0f, v.tileInfo });
		}
		distToCamera = -std::max({ triangle.v0.position.z, triangle.v1.position.z, triangle.v2.position.z });

		// Back face culling (happening in camera space)
		if (IsTriangleBackFacing(triangle))
			continue;

		// Near plane clipping (happening in camera space)
		Triangle clipped[2];
		int nClipped = ClipTriangleNearPlane(triangle, clipped, camera.nearPlane);

		// Compute light intensity for the triangle
		if (mesh.shade)
		{
			triangle.lightIntensity = ComputeLightIntensityForTriangle(
				triangle,
				transformedDirectionalLight,
				m_AmbientLight
			);
		}
		else triangle.lightIntensity = 1.0f;

		for (size_t c = 0; c < nClipped; c++)
		{
			// Loop over vertices and project them
			for (int j = 0; j < 3; j++)
			{
				// Clipped vertex
				Vertex clippedVertex = clipped[c].GetVertexByIndex(j);

				// Transform the clipped vertex to clip space
				Vec4f perspectiveVertex = perspectiveMatrix * Vec4f::FromVec3f(clippedVertex.position, 1.0f);
				float invW = 1.0f / perspectiveVertex.w;

				// Perspective divide
				perspectiveVertex.x *= invW;
				perspectiveVertex.y *= invW;
				perspectiveVertex.z *= invW;

				clippedVertex.u *= invW;
				clippedVertex.v *= invW;

				Vertex screenVertex{ {}, clippedVertex.normal, clippedVertex.u, clippedVertex.v };

				// Convert to screen space
				screenVertex.position.x = (perspectiveVertex.x * 0.5f + 0.5f) * m_Width;
				screenVertex.position.y = (1.0f - (perspectiveVertex.y * 0.5f + 0.5f)) * m_Height;
				screenVertex.position.z = perspectiveVertex.z;
				screenVertex.invW = invW;

				triangle.SetVertexByIndex(j, screenVertex);
			}

			// Screen space back face culling
			if (CalculateTriangleArea(triangle) <= 0)
				continue;

			int left, right, top, bottom;
			ComputeTriangleBoundingBox(triangle, left, right, top, bottom);

			int tileLeft = left / TILE_SIZE;
			int tileRight = right / TILE_SIZE;
			int tileTop = top / TILE_SIZE;
			int tileBottom = bottom / TILE_SIZE;

			int tilesPerRow = (m_Width + TILE_SIZE - 1) / TILE_SIZE;
			int tilesPerCol = (m_Height + TILE_SIZE - 1) / TILE_SIZE;

			tileLeft = std::clamp(tileLeft, 0, tilesPerRow - 1);
			tileRight = std::clamp(tileRight, 0, tilesPerRow - 1);
			tileTop = std::clamp(tileTop, 0, tilesPerCol - 1);
			tileBottom = std::clamp(tileBottom, 0, tilesPerCol - 1);

			ScreenTriangle screenTriangle{ triangle, &texture, tileInfo, distToCamera };
			BoxedScreenTriangle boxedTriangle;
			boxedTriangle.triangle = std::move(screenTriangle);
			boxedTriangle.tileLeft = tileLeft;
			boxedTriangle.tileBottom = tileBottom;
			boxedTriangle.tileRight = tileRight;
			boxedTriangle.tileTop = tileTop;

			out.emplace_back(std::move(boxedTriangle));
		}
	}
}

void Renderer::BuildScreenTileTriangles(
	const Mesh& mesh,
	const Transform& transform,
	const Texture& texture,
	const Camera& camera
)
{
	Mat4 modelMatrix = transform.GetMatrix();
	Mat4 viewMatrix = camera.GetViewMatrix();
	Mat4 perspectiveMatrix = camera.GetPerspectiveMatrix();

	Vec3f transformedDirectionalLight = (viewMatrix * Vec4f::FromVec3f(m_DirectionalLight, 0.0f)).ToVec3f(); // Add .normalized() for a classic directional light

	Triangle triangle;
	AtlasTileInfo tileInfo;
	for (size_t i = 0; i < mesh.indices.size(); i += 3)
	{
		for (size_t j = 0; j < 3; j++)
		{
			// Get the model vertex from the mesh
			const Vertex& v = mesh.vertices[mesh.indices[i + j]];

			tileInfo = v.tileInfo;

			// Transform vertex from model space to world space
			Vec4f worldVertex = modelMatrix * Vec4f(v.position.x, v.position.y, v.position.z, 1.0f);

			// Transform world vertex to camera space
			Vec4f cameraVertex = viewMatrix * worldVertex;

			triangle.SetVertexByIndex(j, { cameraVertex.ToVec3f(), v.normal, v.u, v.v, 0.0f, v.tileInfo });
		}

		// Back face culling (happening in camera space)
		if (IsTriangleBackFacing(triangle))
			continue;

		// Near plane clipping (happening in camera space)
		Triangle clipped[2];
		int nClipped = ClipTriangleNearPlane(triangle, clipped, camera.nearPlane);

		// Compute light intensity for the triangle
		if (mesh.shade)
		{
			triangle.lightIntensity = ComputeLightIntensityForTriangle(
				triangle,
				transformedDirectionalLight,
				m_AmbientLight
			);
		}
		else triangle.lightIntensity = 1.0f;

		for (size_t c = 0; c < nClipped; c++)
		{
			// Loop over vertices and project them
			for (int j = 0; j < 3; j++)
			{
				// Clipped vertex
				Vertex clippedVertex = clipped[c].GetVertexByIndex(j);

				// Transform the clipped vertex to clip space
				Vec4f perspectiveVertex = perspectiveMatrix * Vec4f::FromVec3f(clippedVertex.position, 1.0f);
				float invW = 1.0f / perspectiveVertex.w;

				// Perspective divide
				perspectiveVertex.x *= invW;
				perspectiveVertex.y *= invW;
				perspectiveVertex.z *= invW;

				clippedVertex.u *= invW;
				clippedVertex.v *= invW;

				Vertex screenVertex{ {}, clippedVertex.normal, clippedVertex.u, clippedVertex.v };

				// Convert to screen space
				screenVertex.position.x = (perspectiveVertex.x * 0.5f + 0.5f) * m_Width;
				screenVertex.position.y = (1.0f - (perspectiveVertex.y * 0.5f + 0.5f)) * m_Height;
				screenVertex.position.z = perspectiveVertex.z;
				screenVertex.invW = invW;

				triangle.SetVertexByIndex(j, screenVertex);
			}

			// Screen space back face culling
			if (CalculateTriangleArea(triangle) <= 0) 
				continue;

			int left, right, top, bottom;
			ComputeTriangleBoundingBox(triangle, left, right, top, bottom);

			int tileLeft = left / TILE_SIZE;
			int tileRight = right / TILE_SIZE;
			int tileTop = top / TILE_SIZE;
			int tileBottom = bottom / TILE_SIZE;

			int tilesPerRow = (m_Width + TILE_SIZE - 1) / TILE_SIZE;
			int tilesPerCol = (m_Height + TILE_SIZE - 1) / TILE_SIZE;

			tileLeft = std::clamp(tileLeft, 0, tilesPerRow - 1);
			tileRight = std::clamp(tileRight, 0, tilesPerRow - 1);
			tileTop = std::clamp(tileTop, 0, tilesPerCol - 1);
			tileBottom = std::clamp(tileBottom, 0, tilesPerCol - 1);

			for (int ty = tileTop; ty <= tileBottom; ty++)
				for (int tx = tileLeft; tx <= tileRight; tx++)
				{
					if (tileInfo.isTransparent)
						m_ScreenTiles[ty * tilesPerRow + tx].transparentTriangles.emplace_back(triangle, &texture, tileInfo);
					else
						m_ScreenTiles[ty * tilesPerRow + tx].opaqueTriangles.emplace_back(triangle, &texture, tileInfo);
				}
		}
	}
}

void Renderer::DrawTriangleBarycentricToTile(
	const Triangle& triangle,
	const Texture& texture,
	AtlasTileInfo tileInfo,
	const ScreenTile& tile
)
{
	// Calculate the bounding box
	int left, right, top, bottom;
	ComputeTriangleBoundingBox(triangle, left, right, top, bottom);

	// Clamp to screen bounds
	left = std::max(std::max(left, tile.x0), 0);
	right = std::min(std::min(right, tile.x1), m_Width - 1);
	top = std::max(std::max(top, tile.y0), 0);
	bottom = std::min(std::min(bottom, tile.y1), m_Height - 1);

	if (left > right || top > bottom)
		return;

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
			float px = x + 0.5f;
			float py = y + 0.5f;

			// Barycentric coordinates
			float w0 = (
				(triangle.v1.position.y - triangle.v2.position.y) * (px - triangle.v2.position.x) +
				(triangle.v2.position.x - triangle.v1.position.x) * (py - triangle.v2.position.y)
				) / divisor;

			float w1 = (
				(triangle.v2.position.y - triangle.v0.position.y) * (px - triangle.v2.position.x) +
				(triangle.v0.position.x - triangle.v2.position.x) * (py - triangle.v2.position.y)
				) / divisor;

			float w2 = 1.0f - w0 - w1;

			// If the pixel is inside the triangle
			float eps = 0.0f;
			if (w0 >= eps && w1 >= eps && w2 >= eps)
			{
				float z = w0 * triangle.v0.position.z + w1 * triangle.v1.position.z + w2 * triangle.v2.position.z;

				// Early depth rejection
				if (y * m_Width + x < m_Width * m_Height && z >= m_DepthBuffer[y * m_Width + x])
					continue;

				float invW = w0 * triangle.v0.invW + w1 * triangle.v1.invW + w2 * triangle.v2.invW;
				float u = (w0 * triangle.v0.u + w1 * triangle.v1.u + w2 * triangle.v2.u) / invW;
				float v = (w0 * triangle.v0.v + w1 * triangle.v1.v + w2 * triangle.v2.v) / invW;

				if (m_PerPixelLighting)
				{
					Vec3f normal = (triangle.v0.normal * w0 + triangle.v1.normal * w1 + triangle.v2.normal * w2).normalized();
					uint32_t textureColor = texture.GetPixelAtUV(u, v);
					DrawDepthPixel(x, y, z, ComputeShadedColorForNormal(normal, m_DirectionalLight, m_AmbientLight, textureColor));
				}
				else
				{
					u -= static_cast<int>(u);
					v -= static_cast<int>(v);

					float insetU = 0.01f / static_cast<float>(tileInfo.atlasWidth);
					float insetV = 0.01f / static_cast<float>(tileInfo.atlasHeight);

					float atlasU = tileInfo.u0 + insetU + u * (tileInfo.uScale - 2.0f * insetU);
					float atlasV = tileInfo.v0 + insetV + v * (tileInfo.vScale - 2.0f * insetV);

					uint32_t color = texture.GetPixelAtUV(atlasU, atlasV);
					if (tileInfo.isTransparent && ((color >> 24) & 0xFF) != 0xFF)
					{
						DrawDepthPixelAlphaBlend(x, y, z, ShadeColor(color, triangle.lightIntensity), false);
					}
					else if (tileInfo.isCutout && ((color >> 24) & 0xFF) == 0x00)
						continue;
					else
						DrawDepthPixel(x, y, z, ShadeColor(color, triangle.lightIntensity));
				}
			}
		}
	}
}

void Renderer::DrawTriangleWireframeToTile(
	const Triangle& triangle, 
	const ScreenTile& tile,
	uint32_t color
)
{
	// Calculate the bound box
	int left, right, top, bottom;
	ComputeTriangleBoundingBox(triangle, left, right, top, bottom);

	// Clamp to screen bounds
	left = std::max(std::max(left, tile.x0), 0);
	right = std::min(std::min(right, tile.x1), m_Width - 1);
	top = std::max(std::max(top, tile.y0), 0);
	bottom = std::min(std::min(bottom, tile.y1), m_Height - 1);

	if (left > right || top > bottom)
		return;

	for (int i = 0; i < 3; i++)
	{
		if (triangle.GetVertexByIndex(i).position.x < 0 || triangle.GetVertexByIndex(i).position.x >= m_Width ||
			triangle.GetVertexByIndex(i).position.y < 0 || triangle.GetVertexByIndex(i).position.y >= m_Height)
			return;
	}

	DrawLine(triangle.v0.position, triangle.v1.position, color, 1);
	DrawLine(triangle.v1.position, triangle.v2.position, color, 1);
	DrawLine(triangle.v2.position, triangle.v0.position, color, 1);
}

// Triangle math

inline Triangle Renderer::SortTriangleVerticesByY(const Triangle& triangle)
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

inline void Renderer::ComputeTriangleBoundingBox(const Triangle& triangle, int& left, int& right, int& top, int& bottom)
{
	left = static_cast<int>(
		std::floorf(std::min({ triangle.v0.position.x, triangle.v1.position.x, triangle.v2.position.x }))
	);
	right = static_cast<int>(
		std::ceilf(std::max({ triangle.v0.position.x, triangle.v1.position.x, triangle.v2.position.x }))
	);
	top = static_cast<int>(
		std::floorf(std::min({ triangle.v0.position.y, triangle.v1.position.y, triangle.v2.position.y }))
	);
	bottom = static_cast<int>(
		std::ceilf(std::max({ triangle.v0.position.y, triangle.v1.position.y, triangle.v2.position.y }))
	);
}

inline bool Renderer::IsTriangleBackFacing(const Triangle& triangle)
{
	Vec3f e1 = triangle.v1.position - triangle.v0.position;
	Vec3f e2 = triangle.v2.position - triangle.v0.position;

	Vec3f normal = e1.Cross(e2);
	
	return normal.Dot(-triangle.v0.position) >= 0.0f && normal.z >= 0.0f;
}

inline Vertex Renderer::IntersectWithNearPlane(const Vertex& v0, const Vertex& v1, float nearPlane)
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
	Vertex output[4];
	int outCount = 0;

	for (int i = 0; i < 3; i++)
	{
		Vertex current = triangle.GetVertexByIndex(i);
		Vertex next = triangle.GetVertexByIndex((i + 1) % 3);

		bool currInside = current.position.z < -nearPlane;
		bool nextInside = next.position.z < -nearPlane;

		if (currInside && nextInside) output[outCount++] = next;
		else if (currInside && !nextInside) output[outCount++] = IntersectWithNearPlane(current, next, nearPlane);
		else if (!currInside && nextInside)
		{
			output[outCount++] = IntersectWithNearPlane(current, next, nearPlane);
			output[outCount++] = next;
		}
	}

	if (outCount == 3)
	{
		clipped[0] = { output[0], output[1], output[2] };
		return 1;
	}
	else if (outCount == 4)
	{
		clipped[0] = { output[0], output[1], output[2] };
		clipped[1] = { output[0], output[2], output[3] };
		return 2;
	}

	return 0;
}

inline float Renderer::CalculateTriangleArea(const Triangle& triangle)
{
	return (triangle.v1.position.x - triangle.v0.position.x) * 
		(triangle.v2.position.y - triangle.v0.position.y) -
		(triangle.v1.position.y - triangle.v0.position.y) * 
		(triangle.v2.position.x - triangle.v0.position.x);
}

// Color, light and shading

inline uint32_t Renderer::ShadeColor(uint32_t color, float intensity)
{
	intensity = std::fminf(std::fmaxf(intensity, 0.0f), 1.0f);

	uint8_t a = (color >> 24) & 0xFF;
	uint8_t r = (color >> 16) & 0xFF;
	uint8_t g = (color >> 8) & 0xFF;
	uint8_t b = color & 0xFF;

	r = (uint8_t)(r * intensity);
	g = (uint8_t)(g * intensity);
	b = (uint8_t)(b * intensity);

	return (a << 24) | (r << 16) | (g << 8) | b;
}

inline uint32_t Renderer::ComputeShadedColorForNormal(Vec3f normal, Vec3f lightDir, float ambientLight, uint32_t color)
{
	float intensity = normal.Dot(lightDir);
	intensity = std::fminf(std::fmaxf(intensity, 0.0f) + ambientLight, 1.0f);

	return ShadeColor(color, intensity);
}

inline uint32_t Renderer::ComputeShadedColorForTriangle(const Triangle& triangle, Vec3f lightDir, float ambientLight, uint32_t color)
{
	Vec3f normal = triangle.v0.position.Cross(triangle.v1.position).normalized();
	float intensity = normal.Dot(lightDir);
	intensity = std::fminf(std::fmaxf(intensity, 0.0f) + ambientLight, 1.0f);

	return ShadeColor(color, intensity);
}

inline float Renderer::ComputeLightIntensityForTriangle(const Triangle& triangle, Vec3f lightDir, float ambientLight)
{
	Vec3f e1 = triangle.v0.position - triangle.v1.position;
	Vec3f e2 = triangle.v0.position - triangle.v2.position;

	Vec3f normal = e1.Cross(e2).normalized();
	float intensity = normal.Dot(lightDir);
	return std::fminf(std::fmaxf(intensity, 0.0f) + ambientLight, 1.0f);
}

// Drawing flat triangles and scanlines

void Renderer::DrawFlatBottomTriangle(
	float x0, float y0, float z0, float u0, float v0, float xn0, float yn0, float zn0, float invW0,
	float x1, float y1, float z1, float u1, float v1, float xn1, float yn1, float zn1, float invW1,
	float x2, float y2, float z2, float u2, float v2, float xn2, float yn2, float zn2, float invW2,
	const Texture& texture, float lightIntensity, AtlasTileInfo tileInfo
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

	int y_start = static_cast<int>(std::ceilf(y0 - 0.5f));
	int y_end = static_cast<int>(std::ceilf(y1 - 0.5f));

	for (int y = y_start; y < y_end; y++)
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

		DrawHorizontalDepthTextureLine(xl, xr, y, zl, zr, ul, ur, vl, vr, xnl, xnr, ynl, ynr, znl, znr, invWl, invWr, texture, lightIntensity, tileInfo);
	}
}

void Renderer::DrawFlatTopTriangle(
	float x0, float y0, float z0, float u0, float v0, float xn0, float yn0, float zn0, float invW0,
	float x1, float y1, float z1, float u1, float v1, float xn1, float yn1, float zn1, float invW1,
	float x2, float y2, float z2, float u2, float v2, float xn2, float yn2, float zn2, float invW2,
	const Texture& texture, float lightIntensity, AtlasTileInfo tileInfo
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

	int y_start = static_cast<int>(std::ceilf(y0 - 0.5f));
	int y_end = static_cast<int>(std::ceilf(y2 - 0.5f));

	for (int y = y_start; y < y_end; y++)
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

		DrawHorizontalDepthTextureLine(xl, xr, y, zl, zr, ul, ur, vl, vr, xnl, xnr, ynl, ynr, znl, znr, invWl, invWr, texture, lightIntensity, tileInfo);
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
	const Texture& texture,
	float lightIntensity,
	AtlasTileInfo tileInfo
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

		if (m_PerPixelLighting)
		{
			Vec3f normal = Vec3f(xn, yn, zn).normalized();
			uint32_t textureColor = texture.GetPixelAtUV(u / invW, v / invW);
			DrawDepthPixel(x, y, z, ComputeShadedColorForNormal(normal, m_DirectionalLight, m_AmbientLight, textureColor));
		}
		else
		{
			float uLocal = u / invW;
			float vLocal = v / invW;

			uLocal -= std::floorf(uLocal);
			vLocal -= std::floorf(vLocal);

			float atlasU = tileInfo.u0 + uLocal * tileInfo.uScale;
			float atlasV = tileInfo.v0 + vLocal * tileInfo.vScale;

			DrawDepthPixel(x, y, z, ShadeColor(texture.GetPixelAtUV(atlasU, atlasV), lightIntensity));
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
		if (m_PerPixelLighting)
		{
			Vec3f normal = Vec3f(xn, yn, zn).normalized();
			uint32_t textureColor = texture.GetPixelAtUV(u / invW, v / invW);
			DrawDepthPixel(x, y, z, ComputeShadedColorForNormal(normal, m_DirectionalLight, m_AmbientLight, textureColor));
		}
		else
		{
			float uLocal = u / invW;
			float vLocal = v / invW;

			uLocal -= std::floorf(uLocal);
			vLocal -= std::floorf(vLocal);

			constexpr float inset = 0.01f / 160.f;

			float atlasU = tileInfo.u0 + inset + uLocal * (tileInfo.uScale - 2.0f * inset);
			float atlasV = tileInfo.v0 + inset + vLocal * (tileInfo.vScale - 2.0f * inset);

			atlasU = std::clamp(atlasU, tileInfo.u0 + inset, tileInfo.u0 + tileInfo.uScale - inset);
			atlasV = std::clamp(atlasV, tileInfo.v0 + inset, tileInfo.v0 + tileInfo.vScale - inset);
				
			//DrawDepthPixel(x, y, z, ShadeColor(0xFF00FF00, 1));
			DrawDepthPixel(x, y, z, ShadeColor(texture.GetPixelAtUV(atlasU, atlasV), lightIntensity));
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
}
