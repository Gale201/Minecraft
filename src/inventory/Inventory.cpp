#include "Inventory.h"
#include "../items/ItemRegistry.h"

namespace Minecraft
{
	void Inventory::Render(Renderer& renderer, const ItemRegistry& itemRegistry)
	{
		if (!m_InventoryTexture) return;

		// Render the inventory background
		int textureHeight = renderer.GetHeight() * 11 / 16;
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

		// Render items
		float ratioX = static_cast<float>(textureWidth) / static_cast<float>(m_InventoryTexture->GetWidth());
		float ratioY = static_cast<float>(textureHeight) / static_cast<float>(m_InventoryTexture->GetHeight());

		float startX = std::ceilf(renderer.GetWidth() / 2 - textureWidth / 2 + m_SlotsTopLeftPoint.x * ratioX);
		float startY = std::ceilf(renderer.GetHeight() / 2 - textureHeight / 2 + m_SlotsTopLeftPoint.y * ratioY);

		int j = 3;
		for (size_t i = 0; i < m_Slots.size(); i++)
		{
			const ItemStack& stack = m_Slots[i];

			if (stack.size < 1) continue;

			if (i % 9 == 0 && i != 0)
			{
				++j;
				if (j > 3) j = 0;
			}

			int x = static_cast<int>(startX + ((i % 9) * s_SlotSize + 1) * ratioX);
			int y = static_cast<int>(startY + (j * s_SlotSize + 1 + ((j == 3) ? 4 : 0)) * ratioY);

			Texture itemTexture = itemRegistry.GetTextureForItemType(stack.type);

			int width = static_cast<int>(itemTexture.GetWidth() * ratioX * 0.16f);
			int height = static_cast<int>(itemTexture.GetHeight() * ratioY * 0.16f);

			renderer.DrawTexture(x, y + 1, itemTexture, width, height, false, true);
		}
	}

	void Inventory::RenderHotbar(Renderer& renderer, const ItemRegistry& itemRegistry)
	{
		int textureHeight = renderer.GetHeight() * 1 / 11;
		int textureWidth = textureHeight * m_HotbarTexture->GetWidth() / m_HotbarTexture->GetHeight();

		int hotbarStartY = renderer.GetHeight() - textureHeight - 6;

		renderer.DrawTexture(
			renderer.GetWidth() / 2 - textureWidth / 2,
			hotbarStartY,
			*m_HotbarTexture,
			textureWidth,
			textureHeight,
			true
		);

		// Render hotbar items
		float ratioX = static_cast<float>(textureWidth) / static_cast<float>(m_HotbarTexture->GetWidth());
		float ratioY = static_cast<float>(textureHeight) / static_cast<float>(m_HotbarTexture->GetHeight());

		float startX = std::ceilf(renderer.GetWidth() / 2 - textureWidth / 2) + ratioY * 3;
		float startY = ratioY * 3;

		for (size_t i = 0; i < 9; i++)
		{
			const ItemStack& stack = m_Slots[i];
			
			if (stack.size < 1) continue;

			Texture itemTexture = itemRegistry.GetTextureForItemType(stack.type);

			int width = static_cast<int>(itemTexture.GetWidth() * ratioX * 0.16f);
			int height = static_cast<int>(itemTexture.GetHeight() * ratioY * 0.16f);

			int x = static_cast<int>(std::ceilf(startX + (i % 9) * (s_HotbarSlotSize + 4) * ratioX));
			int y = static_cast<int>(std::ceilf(startY));

			renderer.DrawTexture(x, hotbarStartY + y + 1, itemTexture, width, height, false, true);
		}

		// Render selected slot
		float selectedX = std::ceilf(startX - ratioX * 4 + (m_SelectedHotbarSlotIndex % 9) * (s_HotbarSlotSize + 4) * ratioX);
		float selectedY = std::ceilf(startY - ratioY * 4);

		renderer.DrawTexture(
			static_cast<int>(selectedX),
			hotbarStartY + static_cast<int>(selectedY),
			*m_SelectedSlotTexture,
			static_cast<int>(m_SelectedSlotTexture->GetWidth() * ratioX),
			static_cast<int>(m_SelectedSlotTexture->GetHeight() * ratioY),
			true
		);
	}

	ItemStack Inventory::AddItemStack(ItemStack stack)
	{
		for (int i = 0; i < s_Size; i++)
		{
			ItemStack& slot = GetSlot(i);
			int stackSize = ItemRegistry::GetStackSizeForItemType(stack.type);
			if ((slot.type == stack.type && slot.size < stackSize) || slot.size == 0)
			{
				int spaceLeft = stackSize - slot.size;
				int toAdd = std::min(spaceLeft, static_cast<int>(stack.size));
				slot.size += toAdd;
				slot.type = stack.type;
				stack.size -= toAdd;
			}
		}

		return stack;
	}

	int Inventory::FindFirstEmptySlot() const
	{
		for (int i = 0; i < s_Size; i++)
		{
			if (GetSlot(i).size == 0) return i;
		}
		return -1;
	}

	bool Inventory::CanAddItemStack(const ItemStack& stack) const
	{
		int spaceNeeded = stack.size;
		int stackSize = ItemRegistry::GetStackSizeForItemType(stack.type);
		for (int i = 0; i < s_Size; i++)
		{
			const ItemStack& slot = GetSlot(i);
			if (slot.type == stack.type)
			{
				int spaceLeft = stackSize - slot.size;
				spaceNeeded -= spaceLeft;
				if (spaceNeeded <= 0) return true;
			}
			else if (slot.size == 0)
			{
				spaceNeeded -= stackSize;
				if (spaceNeeded <= 0) return true;
			}
		}
		return false;
	}
}