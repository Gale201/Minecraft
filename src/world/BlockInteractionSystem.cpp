#include "BlockInteractionSystem.h"
#include "../timer/Timer.h"

namespace Minecraft
{
	void BlockInteractionSystem::Initialize(World& world)
	{
		m_World = &world;
	}

	void BlockInteractionSystem::Update(float deltaTime, const Player& player)
	{

	}

	void BlockInteractionSystem::Render(const Player& player, Renderer& renderer)
	{
		WorldRaycastResult raycast;
		if (m_World->Raycast(player.GetCamera().position, player.GetCamera().GetForward(), player.GetMaxBreakBlockDistance(), raycast))
		{
			Transform t;
			t.SetScale(Vec3f(1.0001f, 1.0001f, 1.0001f));
			t.SetPosition(raycast.blockPosition.toVec3f() + Vec3f(0.5f, 0.5f, 0.5f));
			renderer.DrawCubeWireframe(t, m_SelectedBlockHighlightColor, player.GetCamera(), 2, true);
		}
	}

	bool BlockInteractionSystem::BreakBlock(const Player& player)
	{
		WorldRaycastResult raycast;
		if (m_World->Raycast(player.GetCamera().position, player.GetCamera().GetForward(), player.GetMaxBreakBlockDistance(), raycast))
		{
			m_World->SetBlock(raycast.blockPosition, BlockType::Air);
			return true;
		}

		return false;
	}

	bool BlockInteractionSystem::PlaceBlock(const Player& player, Block block)
	{
		WorldRaycastResult raycast;
		if (m_World->Raycast(player.GetCamera().position, player.GetCamera().GetForward(), player.GetMaxPlaceBlockDistance(), raycast))
		{
			Vec3i normal = Vec3i(
				static_cast<int>(raycast.normal.x),
				static_cast<int>(raycast.normal.y),
				static_cast<int>(raycast.normal.z)
			);
			m_World->SetBlock(raycast.blockPosition + normal, BlockType::Glass);
			return true;
		}
		return false;
	}
}
