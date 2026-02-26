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
			t.SetScale(raycast.blockSize);
			t.Scale(Vec3f(1.001f, 1.001f, 1.001f));
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
			Vec3i normal = Vec3i::from(raycast.normal);
			if (player.IsInBlock(raycast.blockPosition + normal))
			{
				return false;
			}
			m_World->SetBlock(raycast.blockPosition + normal, BlockType::OakLeaves);
			return true;
		}
		return false;
	}
}
