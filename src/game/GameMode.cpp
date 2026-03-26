#include "GameMode.h"

namespace Minecraft
{
	bool GameMode::CanInstantBreakBlocks() const
	{
		return m_Type == GameModeType::Creative;
	}

	bool GameMode::BreakingBlocksDropsItems() const
	{ 
		return m_Type == GameModeType::Survival;
	}

	bool GameMode::PlacingBlocksConsumesItems() const
	{
		return m_Type == GameModeType::Survival;
	}

	bool GameMode::CanPlayerFly() const
	{
		return m_Type == GameModeType::Creative;
	}

	bool GameMode::HasHealth() const
	{
		return m_Type == GameModeType::Survival;
	}
}