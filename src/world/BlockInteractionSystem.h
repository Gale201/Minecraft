#include "../src/world/World.h"
#include "../src/player/Player.h"
#include "../render/Renderer.h"

class BlockInteractionSystem
{
public:
	BlockInteractionSystem(World& world) : m_World(world)
	{
	}

	void Update(float deltaTime, const Player& player);
	void Render(const Player& player, Renderer& renderer);

	bool BreakBlock(const Player& player);
	bool PlaceBlock(const Player& player, Block block);

private:
	World& m_World;

	uint32_t m_SelectedBlockHighlightColor = 0x88222222;
};