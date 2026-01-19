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

	void DrawTriangle(const Triangle& triangle, uint32_t color);
	void DrawTriangleWireframe(const Triangle& triangle, uint32_t color);

	void DrawRenderObject(const RenderObject& renderObject, const Camera& camera);

	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }
	uint32_t* GetFrameBuffer() const { return m_FrameBuffer; }

private:
	void ComputeTriangleBoundingBox(const Triangle& triangle, int& left, int& right, int& top, int& bottom);
	bool IsTriangleBackFacing(const Triangle& triangle);
	int ClipTriangleNearPlane(const Triangle& triangle, Triangle clipped[2], float nearPlane);

private:

	int m_Width = 0;
	int m_Height = 0;

	uint32_t* m_FrameBuffer = nullptr;
	float* m_DepthBuffer = nullptr;
};