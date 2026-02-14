#pragma once

#include <cstdint>

#include "Triangle.h"
#include "RenderObject.h"
#include "RenderCommand.h"
#include "Camera.h"
#include "../thread/ThreadSystem.h"
#include <thread>
#include <condition_variable>

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

	void DrawTriangleBarycentric(const Triangle& triangle, const Texture& texture, AtlasTileInfo tileInfo);
	void DrawTriangleScanline(const Triangle& triangle, const Texture& texture, AtlasTileInfo tileInfo);
	void DrawTriangleWireframe(const Triangle& triangle, uint32_t color);

	void DrawMesh(
		const Mesh& mesh,
		const Transform& transform,
		const Texture& texture,
		const Camera& camera
	);
	void DrawRenderObject(const RenderObject& renderObject, const Camera& camera);

	void Render(const std::vector<RenderCommand>& renderList, const Camera& camera, ThreadSystem& threadSystem);

	void CreateTiles();

	void SetDirectionalLight(Vec3f direction) { m_DirectionalLight = direction; }

	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }
	uint32_t* GetFrameBuffer() const { return m_FrameBuffer; }

private:
	struct ScreenTriangle
	{
		Triangle triangle;
		const Texture* texture;
		AtlasTileInfo tileInfo;
	};

	struct ScreenTile
	{
		int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
		std::vector<ScreenTriangle> triangles;
	};

	void RasterizeTile(const ScreenTile& tile);
	void BuildScreenTileTriangles(
		const Mesh& mesh,
		const Transform& transform,
		const Texture& texture,
		const Camera& camera
	);
	void DrawTriangleBarycentricToTile(
		const Triangle& triangle, 
		const Texture& texture, 
		AtlasTileInfo tileInfo,
		const ScreenTile& tile
	);

	static Triangle SortTriangleVerticesByY(const Triangle& triangle);
	static void ComputeTriangleBoundingBox(const Triangle& triangle, int& left, int& right, int& top, int& bottom);
	static bool IsTriangleBackFacing(const Triangle& triangle);
	static Vertex IntersectWithNearPlane(const Vertex& v0, const Vertex& v1, float nearPlane);
	static int ClipTriangleNearPlane(const Triangle& triangle, Triangle clipped[2], float nearPlane);
	static uint32_t ShadeColor(uint32_t color, float intensity);
	static uint32_t ComputeShadedColorForNormal(Vec3f normal, Vec3f lightDir, float ambientLight, uint32_t color);
	static uint32_t ComputeShadedColorForTriangle(const Triangle& triangle, Vec3f lightDir, float ambientLight, uint32_t color);
	static float ComputeLightIntensityForTriangle(const Triangle& triangle, Vec3f lightDir, float ambientLight);

	void DrawFlatBottomTriangle(
		float x0, float y0, float z0, float u0, float v0, float xn0, float yn0, float zn0, float invW0,
		float x1, float y1, float z1, float u1, float v1, float xn1, float yn1, float zn1, float invW1,
		float x2, float y2, float z2, float u2, float v2, float xn2, float yn2, float zn2, float invW2,
		const Texture& texture, float lightIntensity, AtlasTileInfo tileInfo
	);

	void DrawFlatTopTriangle(
		float x0, float y0, float z0, float u0, float v0, float xn0, float yn0, float zn0, float invW0,
		float x1, float y1, float z1, float u1, float v1, float xn1, float yn1, float zn1, float invW1,
		float x2, float y2, float z2, float u2, float v2, float xn2, float yn2, float zn2, float invW2,
		const Texture& texture, float lightIntensity, AtlasTileInfo tileInfo
	);

	void DrawHorizontalDepthTextureLine(
		float x0, float x1,
		int y,
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
	);

private:

	int m_Width = 0;
	int m_Height = 0;

	const int TILE_SIZE = 200;

	uint32_t* m_FrameBuffer = nullptr;
	float* m_DepthBuffer = nullptr;

	Vec3f m_DirectionalLight;
	float m_AmbientLight = 0.2f;

	int m_PerPixelLighting = 0;

	std::vector<ScreenTile> m_ScreenTiles;

	const std::vector<RenderCommand>* m_ActiveRenderList = nullptr;
	const Camera* m_ActiveCamera = nullptr;
};