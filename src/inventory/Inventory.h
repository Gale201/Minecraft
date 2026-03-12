#pragma once
#include "../items/ItemStack.h"
#include <assert.h>
#include <array>
#include "../render/Renderer.h"

namespace Minecraft
{
	class Inventory
	{
	public:
		Inventory() {}

		void Render(Renderer& renderer);

		ItemStack& GetSlot(int slot)
		{
			assert(slot >= 0 && slot < s_Size);
			return m_Slots[slot];
		}

		void SetSlot(int slot, ItemStack stack)
		{
			assert(slot >= 0 && slot < s_Size);
			m_Slots[slot] = stack;
		}

		void SetTexture(Texture& texture)
		{
			m_InventoryTexture = &texture;
		}

	private:
		static constexpr int s_Size = 36;
		static constexpr int s_CraftAreaSize = 4;
		std::array<ItemStack, s_Size> m_Slots;
		std::array<ItemStack, s_CraftAreaSize> m_CraftingSlots;

		Texture* m_InventoryTexture = nullptr;
	};
}