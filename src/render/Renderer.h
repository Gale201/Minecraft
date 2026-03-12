#pragma once

#include <cstdint>
#include <thread>
#include <condition_variable>

#include "Triangle.h"
#include "RenderObject.h"
#include "RenderCommand.h"
#include "Camera.h"
#include "../thread/ThreadSystem.h"
#include "../world/BlockFace.h"

/*
 Renderer class handles the 3d rendering of meshes to a frame buffer.
 The convention for camera is that it looks down the negative Z axis.
*/

namespace Minecraft
{
	class Renderer
	{
	public:
		Renderer() = default;
		~Renderer();

		bool Initialize(int width, int height);
		void Clear(uint32_t color);
		inline void DrawPixel(int x, int y, uint32_t color);
		inline void DrawDepthPixel(int x, int y, float z, uint32_t color);
		inline void DrawPixelAlphaBlend(int x, int y, uint32_t color);
		inline void DrawDepthPixelAlphaBlend(int x, int y, float z, uint32_t color, bool writeDepth = true);

		void DrawLine(Vec3f start, Vec3f end, uint32_t color, int width, bool alphaBlend = false);

		void DrawTriangleBarycentric(const Triangle& triangle, const Texture& texture, AtlasTileInfo tileInfo);
		void DrawTriangleScanline(const Triangle& triangle, const Texture& texture, AtlasTileInfo tileInfo);
		void DrawTriangleWireframe(const Triangle& triangle, uint32_t color);

		void DrawCubeWireframe(const Transform& transform, uint32_t color, const Camera& camera, int width, bool alphaBlend = false, bool centered = true);

		void DrawMesh(
			const Mesh& mesh,
			const Transform& transform,
			const Texture& texture,
			const Camera& camera
		);
		void DrawRenderObject(const RenderObject& renderObject, const Camera& camera);

		void DrawTexture(int x, int y, const Texture& texture, bool alphaBlend = false);

		void DrawTexture(int x, int y, const Texture& texture, int width, int height, bool alphaBlend = false, bool cutoff = false);

		void Render(const std::vector<RenderCommand>& renderList, const Camera& camera, Core::ThreadSystem& threadSystem);

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
			float distToCamera = 0.0f;
		};

		struct ScreenTile
		{
			int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
			std::vector<ScreenTriangle> opaqueTriangles;
			std::vector<ScreenTriangle> transparentTriangles;
		};

		struct BoxedScreenTriangle
		{
			ScreenTriangle triangle;
			int tileLeft, tileRight, tileTop, tileBottom;
		};

		void RasterizeTile(ScreenTile& tile);

		void BuildScreenTriangles(
			const Mesh& mesh,
			const Transform& transform,
			const Texture& texture,
			const Camera& camera,
			std::vector<BoxedScreenTriangle>& out
		);
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
		void DrawTriangleWireframeToTile(
			const Triangle& triangle,
			const ScreenTile& tile,
			uint32_t color
		);

		inline static Triangle SortTriangleVerticesByY(const Triangle& triangle);
		inline static void ComputeTriangleBoundingBox(const Triangle& triangle, int& left, int& right, int& top, int& bottom);
		inline static bool IsTriangleBackFacing(const Triangle& triangle);
		inline static Vertex IntersectWithNearPlane(const Vertex& v0, const Vertex& v1, float nearPlane);
		static int ClipTriangleNearPlane(const Triangle& triangle, Triangle clipped[2], float nearPlane);
		inline static float CalculateTriangleArea(const Triangle& triangle);

		inline static uint32_t ShadeColor(uint32_t color, float intensity);
		inline static uint32_t ComputeShadedColorForNormal(Vec3f normal, Vec3f lightDir, float ambientLight, uint32_t color);
		inline static uint32_t ComputeShadedColorForTriangle(const Triangle& triangle, Vec3f lightDir, float ambientLight, uint32_t color);
		inline static float ComputeLightIntensityForTriangle(const Triangle& triangle, Vec3f lightDir, float ambientLight);

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

		const int TILE_SIZE = 256;

		uint32_t* m_FrameBuffer = nullptr;
		float* m_DepthBuffer = nullptr;

		Vec3f m_DirectionalLight;
		float m_AmbientLight = 0.4f;

		int m_PerPixelLighting = 0;

		std::vector<ScreenTile> m_ScreenTiles;

		const std::vector<RenderCommand>* m_ActiveRenderList = nullptr;
		const Camera* m_ActiveCamera = nullptr;
	};
}
