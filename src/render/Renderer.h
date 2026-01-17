#pragma once

#include <cstdint>
#include "Triangle.h"

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
	void DrawPixel(int x, int y, uint32_t color);
	void DrawDepthPixel(int x, int y, float z, uint32_t color);

	void DrawTriangle(const Triangle& triangle, uint32_t color);

	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }
	uint32_t* GetFrameBuffer() const { return m_FrameBuffer; }

private:

	int m_Width = 0;
	int m_Height = 0;

	uint32_t* m_FrameBuffer = nullptr;
	float* m_DepthBuffer = nullptr;

	void ComputeTriangleBoundingBox(const Triangle& triangle, int& left, int& right, int& top, int& bottom);
};