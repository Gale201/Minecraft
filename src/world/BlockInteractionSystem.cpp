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
			BlockBoundingBox box = GetBlockBoundingBox(raycast.blockType);
			t.SetScale(box.size);
			t.Scale(Vec3f(1.001f, 1.001f, 1.001f));
			t.SetPosition(raycast.blockPosition.toVec3f() + box.size * 0.5f + box.offset);
			renderer.DrawCubeWireframe(t, m_SelectedBlockHighlightColor, player.GetCamera(), 2, true);
		}
	}

	bool BlockInteractionSystem::BreakBlock(const Player& player)
	{
		WorldRaycastResult raycast;
		if (m_World->Raycast(player.GetCamera().position, player.GetCamera().GetForward(), player.GetMaxBreakBlockDistance(), raycast))
		{
			Vec3f itemPos = raycast.blockPosition.toVec3f() + Vec3f(0.4f, 0.4f, 0.4f);
			m_World->SetBlock(raycast.blockPosition, BlockType::Air);
			m_World->SpawnItem(
				itemPos,
				{ ItemRegistry::GetItemTypeFromBlockType(raycast.blockType), 1 }
			);
			return true;
		}

		return false;
	}

	bool BlockInteractionSystem::PlaceBlock(Player& player, Block block)
	{
		WorldRaycastResult raycast;
		if (m_World->Raycast(player.GetCamera().position, player.GetCamera().GetForward(), player.GetMaxPlaceBlockDistance(), raycast))
		{
			Vec3i normal = Vec3i::from(raycast.normal);
			BlockType type = player.GetRequestedBlockToPlace().type;

			if (Block(type).IsFullBlock() && player.IsInBlock(raycast.blockPosition + normal))
				return false;

			if (type == BlockType::Undefined)
				return false;
			m_World->SetBlock(raycast.blockPosition + normal, type);
			player.RemoveItemAfterPlacing();
			return true;
		}
		return false;
	}
}
