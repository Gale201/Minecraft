#pragma once

#include <assert.h>
#include <array>

#include "../items/ItemStack.h"
#include "../items/ItemRegistry.h"
#include "../render/Renderer.h"

namespace Minecraft
{
	class Inventory
	{
	public:
		static constexpr int s_Size = 36;
		static constexpr int s_CraftAreaSize = 4;
		static constexpr int s_SlotSize = 18;
		static constexpr int s_HotbarSlotSize = 16;

	public:
		Inventory() {}

		void Render(Renderer& renderer, const ItemRegistry& itemRegistry);
		void RenderHotbar(Renderer& renderer, const ItemRegistry& itemRegistry);

		ItemStack AddItemStack(ItemStack stack);
		int FindFirstEmptySlot() const;
		bool CanAddItemStack(const ItemStack& stack) const;

		ItemStack& GetSlot(int slot)
		{
			assert(slot >= 0 && slot < s_Size);
			return m_Slots[slot];
		}

		const ItemStack& GetSlot(int slot) const
		{
			assert(slot >= 0 && slot < s_Size);
			return m_Slots[slot];
		}

		std::array<ItemStack, s_Size>& GetSlots() { return m_Slots; }
		const std::array<ItemStack, s_Size>& GetSlots() const { return m_Slots; }

		ItemStack& GetSelectedHotbarSlot() { return m_Slots[m_SelectedHotbarSlotIndex]; }
		const ItemStack& GetSelectedHotbarSlot() const { return m_Slots[m_SelectedHotbarSlotIndex]; }
		int GetSelectedHotbarSlotIndex() const { return m_SelectedHotbarSlotIndex; }

		void SetSlot(int slot, ItemStack stack)
		{
			assert(slot >= 0 && slot < s_Size);
			m_Slots[slot] = stack;
		}

		void SetSelectedHotbarSlotIndex(int slot)
		{
			assert(slot >= 0 && slot < 9);
			m_SelectedHotbarSlotIndex = slot;
		}

		void SetTextures(Texture& inventroyTexture, Texture& hotbarTexture, Texture& selectedTexture)
		{
			m_InventoryTexture = &inventroyTexture;
			m_HotbarTexture = &hotbarTexture;
			m_SelectedSlotTexture = &selectedTexture;
		}

	private:
		std::array<ItemStack, s_Size> m_Slots;
		std::array<ItemStack, s_CraftAreaSize> m_CraftingSlots;

		Vec2f m_SlotsTopLeftPoint{ 7.f, 83.f };

		int m_SelectedHotbarSlotIndex = 0;

		Texture* m_InventoryTexture = nullptr;
		Texture* m_HotbarTexture = nullptr;
		Texture* m_SelectedSlotTexture = nullptr;
	};
}