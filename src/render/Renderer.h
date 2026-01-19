#pragma once

#include <cstdint>

#include "Triangle.h"
#include "RenderObject.h"
#include "Camera.h"

/*
 Renderer class handles the 3d rendering of meshes to a frame buffer.
 The convention for camera is that it looks down the negative Z axis.
*/
class Renderer
{
public:
	Renderer() = default;
	~Renderer();

	bool Initialize(int width, int height);
	void Clear(uint32_t color);
	inline void DrawPixel(int x, int y, uint32_t color);
	inline void DrawDepthPixel(int x, int y, float z, uint32_t color);

	void DrawTriangleBarycentric(const Triangle& triangle, const Texture& texture);
	void DrawTriangleScanline(const Triangle& triangle, const Texture& texture);
	void DrawTriangleWireframe(const Triangle& triangle, uint32_t color);

	void DrawRenderObject(const RenderObject& renderObject, const Camera& camera);

	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }
	uint32_t* GetFrameBuffer() const { return m_FrameBuffer; }

private:
	Triangle SortTriangleVerticesByY(const Triangle& triangle);
	void ComputeTriangleBoundingBox(const Triangle& triangle, int& left, int& right, int& top, int& bottom);
	bool IsTriangleBackFacing(const Triangle& triangle);
	Vertex IntersectWithNearPlane(const Vertex& v0, const Vertex& v1, float nearPlane);
	int ClipTriangleNearPlane(const Triangle& triangle, Triangle clipped[2], float nearPlane);

	void DrawFlatBottomTriangle(float x0, float y0, float z0, float u0, float v0, float xn0, float yn0, float zn0,
		float x1, float y1, float z1, float u1, float v1, float xn1, float yn1, float zn1,
		float x2, float y2, float z2, float u2, float v2, float xn2, float yn2, float zn2,
		Texture texture);
	void DrawFlatTopTriangle(float x0, float y0, float z0, float u0, float v0, float xn0, float yn0, float zn0,
		float x1, float y1, float z1, float u1, float v1, float xn1, float yn1, float zn1,
		float x2, float y2, float z2, float u2, float v2, float xn2, float yn2, float zn2,
		Texture texture);
	void DrawHorizontalDepthTextureLine(
		float x0, float x1,
		int y,
		float inv_z0, float inv_z1,
		float u0_over_z, float u1_over_z,
		float v0_over_z, float v1_over_z,
		float xn0_over_z, float xn1_over_z,
		float yn0_over_z, float yn1_over_z,
		float zn0_over_z, float zn1_over_z,
		Texture texture
	);

	void DrawFlatTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2, const Texture& texture);

private:

	int m_Width = 0;
	int m_Height = 0;

	uint32_t* m_FrameBuffer = nullptr;
	float* m_DepthBuffer = nullptr;
};