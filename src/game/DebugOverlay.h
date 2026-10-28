#pragma once
#include "../render/Renderer.h"
#include "../render/RendererStats.h"
#include "../world/WorldStats.h"

namespace Minecraft
{
	class DebugOverlay
	{
	public:
		void Toggle() { m_Active = !m_Active; }

		void Update(float fps, float frameTime, float updateTime, float renderTime, const WorldStats& worldStats, const RendererStats& rendererStats, float deltaTime);

		void Render(Renderer& renderer, const FontLoader& fontLoader);

	private:
		bool m_Active = false;

		float m_AvgTimer = 0.0f;
		float m_MaxTimer = 0.0f;
		uint32_t m_FrameCount = 0;

		/* Frame timing */
		float m_FpsSum = 0.0f;
		float m_FrameTimeSum = 0.0f;
		float m_AvgFps = 0.0f;
		float m_AvgFrameTime = 0.0f;

		/* Game timing */
		float m_UpdateTimeSum = 0.0f;
		float m_RenderTimeSum = 0.0f;
		int m_GameFrameCount = 0;
		float m_AvgUpdateTime = 0.0f;
		float m_AvgRenderTime = 0.0f;

		/* Renderer stats */
		RendererStats m_RendererStats;
		float m_AvgRendererBuildTime = 0.0f;
		float m_AvgRendererRasterTime = 0.0f;

		/* World stats */
		WorldStats m_WorldStats;
		float m_MaxWorldMeshTime = 0.0f;
		float m_MaxWorldLoadTime = 0.0f;
	};
}
