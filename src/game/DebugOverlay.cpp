#include "DebugOverlay.h"

#include <format>

namespace Minecraft
{
	void DebugOverlay::Update(float fps, float frameTime, float updateTime, float renderTime, const WorldStats& worldStats, const RendererStats& rendererStats)
	{
		m_FpsSum += fps;
		m_FrameTimeSum += frameTime;

		m_UpdateTimeSum += updateTime;
		m_RenderTimeSum += renderTime;

		m_WorldStats.meshMs = std::max(m_WorldStats.meshMs, worldStats.meshMs);
		m_WorldStats.loadMs = std::max(m_WorldStats.loadMs, worldStats.loadMs);
		m_WorldStats.chunksMeshed += worldStats.chunksMeshed;
		m_WorldStats.chunksLoaded = worldStats.chunksLoaded;
		m_WorldStats.meshQueueSize = worldStats.meshQueueSize;

		m_RendererStats.buildMs += rendererStats.buildMs;
		m_RendererStats.rasterMs += rendererStats.rasterMs;
		m_RendererStats.renderCommands = rendererStats.renderCommands;
		m_RendererStats.trianglesSubmitted = rendererStats.trianglesSubmitted;
		m_RendererStats.trianglesBinned = rendererStats.trianglesBinned;
		m_RendererStats.pixelsTested = rendererStats.pixelsTested;
		m_RendererStats.pixelsDrawn = rendererStats.pixelsDrawn;

		if (m_FrameCount > 30)
		{
			m_AvgFps = m_FpsSum / float(m_FrameCount);
			m_AvgFrameTime = m_FrameTimeSum / float(m_FrameCount);
			m_FpsSum = 0.0f;
			m_FrameTimeSum = 0.0f;

			m_AvgUpdateTime = m_UpdateTimeSum / float(m_FrameCount);
			m_AvgRenderTime = m_RenderTimeSum / float(m_FrameCount);
			m_GameFrameCount = 0;
			m_UpdateTimeSum = 0.0f;
			m_RenderTimeSum = 0.0f;

			m_WorldStats.chunksMeshed = 0;

			m_AvgRendererBuildTime = m_RendererStats.buildMs / float(m_FrameCount);
			m_AvgRendererRasterTime = m_RendererStats.rasterMs / float(m_FrameCount);
			m_RendererStats.buildMs = 0.0f;
			m_RendererStats.rasterMs = 0.0f;

			m_FrameCount = 0;
		}
	}

	void DebugOverlay::Render(Renderer& renderer, const FontLoader& fontLoader)
	{
		m_FrameCount++;

		if (!m_Active)
			return;

		float overdraw = m_RendererStats.pixelsDrawn / (float)(renderer.GetWidth() * renderer.GetHeight());
		float efficiency = m_RendererStats.pixelsDrawn / (float)m_RendererStats.pixelsTested * 100.0f;

		std::string gameStatsText = "fps: " + std::to_string(static_cast<int>(m_AvgFps)) + " (" + std::format("{:.2f}", m_AvgFrameTime) + " ms)\n";
		gameStatsText += "update: " + std::format("{:.2f}", m_AvgUpdateTime) + " ms\n";
		gameStatsText += "render: " + std::format("{:.2f}", m_AvgRenderTime) + " ms";

		// renderer stats
		std::string rendererStatsText = "build: " + std::format("{:.2f}", m_AvgRendererBuildTime) + " ms\n";
		rendererStatsText += "raster: " + std::format("{:.2f}", m_AvgRendererRasterTime) + " ms\n";
		rendererStatsText += "render commands: " + std::to_string(m_RendererStats.renderCommands) + "\n";
		rendererStatsText += "triangles submitted: " + std::to_string(m_RendererStats.trianglesSubmitted) + "\n";
		rendererStatsText += "triangles binned: " + std::to_string(m_RendererStats.trianglesBinned) + "\n";
		rendererStatsText += "pixels tested: " + std::to_string(m_RendererStats.pixelsTested) + "\n";
		rendererStatsText += "pixels drawn: " + std::to_string(m_RendererStats.pixelsDrawn) + "\n";
		rendererStatsText += "overdraw: " + std::format("{:.2f}", overdraw) + "\n";
		rendererStatsText += "efficiency: " + std::format("{:.2f}", efficiency) + "%";

		// world stats
		std::string worldStatsText = "meshing: " + std::format("{:.2f}", m_WorldStats.meshMs) + " ms\n";
		worldStatsText += "loading: " + std::format("{:.2f}", m_WorldStats.loadMs) + " ms\n";
		worldStatsText += "chunks meshed: " + std::to_string(m_WorldStats.chunksMeshed) + "\n";
		worldStatsText += "chunks loaded: " + std::to_string(m_WorldStats.chunksLoaded) + "\n";
		worldStatsText += "mesh queue size: " + std::to_string(m_WorldStats.meshQueueSize);

		renderer.DrawText(gameStatsText, 10, 10, 12, 16, fontLoader, true);
		renderer.DrawText(rendererStatsText, 10, 80, 12, 16, fontLoader, true);
		renderer.DrawText(worldStatsText, renderer.GetWidth() - 250, 10, 12, 16, fontLoader, true);
	}
}