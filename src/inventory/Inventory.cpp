#include "Inventory.h"

namespace Minecraft
{
	void Inventory::Render(Renderer& renderer)
	{
		if (!m_InventoryTexture) return;

		int textureHeight = renderer.GetHeight() * 9 / 10;
		int textureWidth = textureHeight * m_InventoryTexture->GetWidth() / m_InventoryTexture->GetHeight();

		renderer.DrawTexture(
			renderer.GetWidth() / 2 - textureWidth / 2,
			renderer.GetHeight() / 2 - textureHeight / 2,
			*m_InventoryTexture,
			textureWidth,
			textureHeight,
			false,
			true
		);
	}
}