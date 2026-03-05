#pragma once

namespace Minecraft
{
	enum class GameModeType
	{
		Survival,
		Creative
	};

	class GameMode
	{
	public:
		GameMode(GameModeType type) : m_Type(type) {}

		bool CanInstantBreakBlocks() const;
		bool BreakingBlocksDropsItems() const;
		bool CanPlayerFly() const;

		void SetGameModeType(GameModeType type) { m_Type = type; }

	private:
		GameModeType m_Type;
	};
}
