#include "Inventory.h"

namespace Minecraft
{
	void Inventory::Update(const Core::InputManager& inputManager)
	{
		m_ItemToRemove.size = 0;

		m_HoldingStackPosition = Vec2i(inputManager.GetMouseX(), inputManager.GetMouseY());

		int textureHeight = m_RendererHeight * 11 / 16;
		int textureWidth = textureHeight * m_InventoryTexture->GetWidth() / m_InventoryTexture->GetHeight();

		float ratioX = static_cast<float>(textureWidth) / static_cast<float>(m_InventoryTexture->GetWidth());
		float ratioY = static_cast<float>(textureHeight) / static_cast<float>(m_InventoryTexture->GetHeight());

		float startX = std::ceilf(m_RendererWidth / 2 - textureWidth / 2 + m_SlotsTopLeftPoint.x * ratioX);
		float startY = std::ceilf(m_RendererHeight / 2 - textureHeight / 2 + m_SlotsTopLeftPoint.y * ratioY);

		int mouseX = inputManager.GetMouseX();
		int mouseY = inputManager.GetMouseY();

		int selectedSlotX = static_cast<int>((mouseX - startX) / (s_SlotSize * ratioX));
		int selectedSlotY = 1 + static_cast<int>((mouseY - startY) / (s_SlotSize * ratioY));
		if (mouseY >= startY + 3 * s_SlotSize * ratioY + 4 * ratioY && mouseY < startY + 4 * s_SlotSize * ratioY + 4 * ratioY)
			selectedSlotY = 0;

		if (inputManager.IsMouseButtonJustPressed(Core::MouseButton::Left))
		{
			if (selectedSlotX >= 0 && selectedSlotX < 9 && selectedSlotY >= 0 && selectedSlotY < 4)
			{
				int selectedSlotIndex = selectedSlotY * 9 + selectedSlotX;
				if (selectedSlotIndex < s_Size && m_Slots[selectedSlotIndex].size > 0)
				{
					if (m_HoldingStack.size == 0)
					{
						m_HoldingStack = m_Slots[selectedSlotIndex];
						m_Slots[selectedSlotIndex].size = 0;
						m_WaitForMouseRelease = true;
					}
				}
			}
		}
		else if (inputManager.IsMouseButtonJustReleased(Core::MouseButton::Left))
		{
			if (!m_WaitForMouseRelease && selectedSlotX >= 0 && selectedSlotX < 9 && selectedSlotY >= 0 && selectedSlotY < 4)
			{
				int selectedSlotIndex = selectedSlotY * 9 + selectedSlotX;
				if (selectedSlotIndex < s_Size)
				{
					ItemStack& selectedSlot = m_Slots[selectedSlotIndex];

					if (selectedSlot.type == m_HoldingStack.type)
					{
						if (selectedSlot.size + m_HoldingStack.size <= ItemRegistry::GetStackSizeForItemType(selectedSlot.type))
						{
							selectedSlot.size += m_HoldingStack.size;
							m_HoldingStack.size = 0;
						}
						else
						{
							int spaceLeft = ItemRegistry::GetStackSizeForItemType(selectedSlot.type) - selectedSlot.size;
							selectedSlot.size += spaceLeft;
							m_HoldingStack.size -= spaceLeft;
						}
					}
					else
					{
						ItemStack temp = m_HoldingStack;
						m_HoldingStack = selectedSlot;
						selectedSlot = temp;
					}
				}
			}
			else if (!m_WaitForMouseRelease)
			{
				m_ItemToRemove = m_HoldingStack;
				m_HoldingStack.size = 0;
			}

			m_WaitForMouseRelease = false;
		}
		else if (inputManager.IsMouseButtonJustReleased(Core::MouseButton::Right))
		{
			if (!m_WaitForMouseRelease && selectedSlotX >= 0 && selectedSlotX < 9 && selectedSlotY >= 0 && selectedSlotY < 4)
			{
				int selectedSlotIndex = selectedSlotY * 9 + selectedSlotX;
				if (selectedSlotIndex < s_Size)
				{
					ItemStack& selectedSlot = m_Slots[selectedSlotIndex];

					if (
						m_HoldingStack.size > 1 &&
						(selectedSlot.size == 0 ||
						(
							selectedSlot.type == m_HoldingStack.type &&
							selectedSlot.size < ItemRegistry::GetStackSizeForItemType(selectedSlot.type)
						))
					)
					{
						selectedSlot.type = m_HoldingStack.type;
						selectedSlot.size++;
						m_HoldingStack.size--;
					}
					else if (m_HoldingStack.size == 1 && selectedSlot.type == m_HoldingStack.type)
					{
						selectedSlot.size++;
						m_HoldingStack.size = 0;
					}
					else
					{
						ItemStack temp = m_HoldingStack;
						m_HoldingStack = selectedSlot;
						selectedSlot = temp;
					}
				}
			}

			m_WaitForMouseRelease = false;
		}
		
		if (inputManager.IsMouseButtonJustDoublePressed(Core::MouseButton::Left))
		{
			int selectedSlotIndex = selectedSlotY * 9 + selectedSlotX;

			if (
				m_HoldingStack.size > 0 && 
				m_HoldingStack.size < ItemRegistry::GetStackSizeForItemType(m_HoldingStack.type) &&
				selectedSlotIndex < s_Size &&
				m_Slots[selectedSlotIndex].size == 0
			)
			{
				for (int i = 0; i < s_Size; i++)
				{
					if (m_Slots[i].type == m_HoldingStack.type && m_Slots[i].size < ItemRegistry::GetStackSizeForItemType(m_Slots[i].type))
					{
						int spaceLeft = ItemRegistry::GetStackSizeForItemType(m_Slots[i].type) - m_HoldingStack.size;
						if (spaceLeft >= m_Slots[i].size)
						{
							m_HoldingStack.size += m_Slots[i].size;
							m_Slots[i].size = 0;
						}
						else
						{
							m_HoldingStack.size += spaceLeft;
							m_Slots[i].size -= spaceLeft;
						}
					}
				}
			}
		}
	}

	void Inventory::UpdateHotbar(const Core::InputManager& inputManager)
	{
		// Handle hotbar selection input
		for (int i = 1; i < 10; i++)
		{
			int k0 = static_cast<std::underlying_type<Core::Key>::type>(Core::Key::K_0);
			if (inputManager.IsKeyDown(static_cast<Core::Key>(k0 + i)))
			{
				SetSelectedHotbarSlotIndex(i - 1);
				break;
			}
		}

		if (inputManager.GetMouseWheelMotion() > 0)
			m_SelectedHotbarSlotIndex = (m_SelectedHotbarSlotIndex + 8) % 9;
		else if (inputManager.GetMouseWheelMotion() < 0)
			m_SelectedHotbarSlotIndex = (m_SelectedHotbarSlotIndex + 1) % 9;
	}

	void Inventory::Render(Renderer& renderer, const ItemRegistry& itemRegistry, const FontLoader& fontLoader)
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

			if (i % 9 == 0 && i != 0)
			{
				++j;
				if (j > 3) j = 0;
			}

			if (stack.size < 1) continue;

			int x = static_cast<int>(startX + ((i % 9) * s_SlotSize + 1) * ratioX);
			int y = static_cast<int>(startY + (j * s_SlotSize + 1 + ((j == 3) ? 4 : 0)) * ratioY);

			Texture itemTexture = itemRegistry.GetTextureForItemType(stack.type);

			int width = static_cast<int>(itemTexture.GetWidth() * ratioX * 0.16f);
			int height = static_cast<int>(itemTexture.GetHeight() * ratioY * 0.16f);

			RenderItem(renderer, itemRegistry, fontLoader, stack, x, y + 1, ratioX, ratioY);
		}

		if (m_HoldingStack.size > 0)
		{
			Texture itemTexture = itemRegistry.GetTextureForItemType(m_HoldingStack.type);

			int width = static_cast<int>(itemTexture.GetWidth() * ratioX * 0.16f);
			int height = static_cast<int>(itemTexture.GetHeight() * ratioY * 0.16f);

			RenderItem(renderer, itemRegistry, fontLoader, m_HoldingStack, m_HoldingStackPosition.x - width / 2, m_HoldingStackPosition.y - height / 2, ratioX, ratioY);
		}
	}

	void Inventory::RenderHotbar(Renderer& renderer, const ItemRegistry& itemRegistry, const FontLoader& fontLoader)
	{
		m_RendererWidth = renderer.GetWidth();
		m_RendererHeight = renderer.GetHeight();

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

		float ratioX = static_cast<float>(textureWidth) / static_cast<float>(m_HotbarTexture->GetWidth());
		float ratioY = static_cast<float>(textureHeight) / static_cast<float>(m_HotbarTexture->GetHeight());

		float startX = std::ceilf(renderer.GetWidth() / 2 - textureWidth / 2) + ratioY * 3;
		float startY = ratioY * 3;

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

		// Render hotbar items
		for (size_t i = 0; i < 9; i++)
		{
			const ItemStack& stack = m_Slots[i];
			
			if (stack.size < 1) continue;

			int x = static_cast<int>(std::ceilf(startX + (i % 9) * (s_HotbarSlotSize + 4) * ratioX));
			int y = static_cast<int>(std::ceilf(startY)) + hotbarStartY + 1;

			RenderItem(renderer, itemRegistry, fontLoader, stack, x, y, ratioX, ratioY);
		}
	}

	ItemStack Inventory::AddItemStack(ItemStack stack)
	{
		for (int i = 0; i < s_Size; i++)
		{
			ItemStack& slot = GetSlot(i);
			int stackSize = ItemRegistry::GetStackSizeForItemType(stack.type);
			if (slot.size > 0 && slot.type == stack.type && slot.size < stackSize)
			{
				int spaceLeft = stackSize - slot.size;
				int toAdd = std::min(spaceLeft, static_cast<int>(stack.size));
				slot.size += toAdd;
				slot.type = stack.type;
				stack.size -= toAdd;
			}
		}

		if (stack.size == 0) return stack;

		for (int i = 0; i < s_Size; i++)
		{
			ItemStack& slot = GetSlot(i);
			int stackSize = ItemRegistry::GetStackSizeForItemType(stack.type);
			if (slot.size == 0)
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
	
	/* Privatr functions */

	void Inventory::RenderItem(Renderer& renderer, const ItemRegistry& itemRegistry, const FontLoader& fontLoader, const ItemStack& stack, int x, int y, float ratioX, float ratioY)
	{
		Texture itemTexture = itemRegistry.GetTextureForItemType(stack.type);

		int width = static_cast<int>(itemTexture.GetWidth() * ratioX * 0.16f);
		int height = static_cast<int>(itemTexture.GetHeight() * ratioY * 0.16f);

		renderer.DrawTexture(x, y, itemTexture, width, height, false, true);

		if (stack.size > 1)
		{
			std::string itemCountStr = std::to_string(stack.size);
			int charWidth = fontLoader.GetCharWidth() * ratioX;
			int charHeight = fontLoader.GetCharHeight() * ratioY;
			int textX = x + s_SlotSize * ratioX - charWidth * itemCountStr.size();
			int textY = y + s_SlotSize * ratioY - charHeight - ratioY;
			renderer.DrawText(itemCountStr, textX, textY, charWidth, charHeight, fontLoader);
		}
	}
}