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

		m_MouseX = inputManager.GetMouseX();
		m_MouseY = inputManager.GetMouseY();

		int selectedSlotX = static_cast<int>((m_MouseX - startX) / (s_SlotSize * ratioX));
		int selectedSlotY = 1 + static_cast<int>((m_MouseY - startY) / (s_SlotSize * ratioY));
		if (m_MouseY >= startY + 3 * s_SlotSize * ratioY + 4 * ratioY && m_MouseY < startY + 4 * s_SlotSize * ratioY + 4 * ratioY)
			selectedSlotY = 0;

		int selectedSlotIndex = selectedSlotY * 9 + selectedSlotX;

		if (inputManager.IsMouseButtonJustPressed(Core::MouseButton::Left))
		{
			if (selectedSlotX >= 0 && selectedSlotX < 9 && selectedSlotY >= 0 && selectedSlotY < 4)
			{
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
			if (selectedSlotIndex < s_Size)
			{
				ItemStack& selectedSlot = m_Slots[selectedSlotIndex];

				if (m_HoldingStack.size == 0 && selectedSlot.size > 0)
				{
					m_HoldingStack.type = selectedSlot.type;
					m_HoldingStack.size = (selectedSlot.size + 1) / 2;
					selectedSlot.size -= m_HoldingStack.size;
				}
				else if (m_HoldingStack.size > 0 && selectedSlot.size > 0)
				{
					if (m_HoldingStack.type == selectedSlot.type && selectedSlot.size < ItemRegistry::GetStackSizeForItemType(selectedSlot.type))
					{
						selectedSlot.size++;
						m_HoldingStack.size--;
					}
					else if (m_HoldingStack.type != selectedSlot.type)
					{
						ItemStack temp = m_HoldingStack;
						m_HoldingStack = selectedSlot;
						selectedSlot = temp;
					}
				}
				else if (m_HoldingStack.size > 0 && selectedSlot.size == 0)
				{
					selectedSlot.type = m_HoldingStack.type;
					selectedSlot.size = 1;
					m_HoldingStack.size--;
				}
			}
		}
		else if (inputManager.IsMouseButtonJustReleased(Core::MouseButton::Right))
		{
			if (!m_WaitForMouseRelease && selectedSlotX >= 0 && selectedSlotX < 9 && selectedSlotY >= 0 && selectedSlotY < 4)
			{
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
				m_WaitForMouseRelease = true;
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

		// Render hovered slot highlight
		float selectedSlotX = std::floorf((m_MouseX - startX) / (s_SlotSize * ratioX));
		float selectedSlotY = std::floorf((m_MouseY - startY) / (s_SlotSize * ratioY));
		if (m_MouseY >= startY + 3 * s_SlotSize * ratioY + 4 * ratioY && m_MouseY < startY + 4 * s_SlotSize * ratioY + 4 * ratioY)
			selectedSlotY = 3 + 4.f / s_SlotSize;
		if (selectedSlotY > 2 && selectedSlotY < 3.1f) selectedSlotY = -1;
		
		if (selectedSlotX >= 0 && selectedSlotX < 9 && selectedSlotY >= 0 && selectedSlotY < 4)
			renderer.DrawTexture(
				static_cast<int>(startX + selectedSlotX * s_SlotSize * ratioX + ratioX + 1),
				static_cast<int>(startY + selectedSlotY * s_SlotSize * ratioY + ratioY + 1),
				m_SlotHoverTexture,
				static_cast<int>(16.0f * ratioX),
				static_cast<int>(16.0f * ratioY),
				true
			);
		
		// Render the holding item stack
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

		float startX = std::ceilf(renderer.GetWidth() / 2.f - textureWidth / 2.f) + ratioY * 3;
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
	
	void Inventory::RenderHotbarIcons(Renderer& renderer, int health, int armor)
	{
		int textureHeight = renderer.GetHeight() * 1 / 11;
		int textureWidth = textureHeight * m_HotbarTexture->GetWidth() / m_HotbarTexture->GetHeight();

		int hotbarStartY = renderer.GetHeight() - textureHeight - 6;
		
		float ratioX = static_cast<float>(textureWidth) / static_cast<float>(m_HotbarTexture->GetWidth());
		float ratioY = static_cast<float>(textureHeight) / static_cast<float>(m_HotbarTexture->GetHeight());

		float startX = std::ceilf(renderer.GetWidth() / 2.f - textureWidth / 2.f) + ratioY * 3;
		float startY = ratioY * 3;

		// Render hearths
		Texture borderTexture = m_IconsTexture->Crop(16, 0, 9, 9);
		for (int i = 0; i < 20; i += 2)
		{
			int idx = 4;
			if (health - 1 == i) idx = 5;
			else if (health < i + 2) idx = 0;

			Texture heartTexture = m_IconsTexture->Crop(16 + idx * 9, 0, 9, 9);

			int x = static_cast<int>(std::floorf(startX - ratioX * 3 + ((i / 2) % 10) * 8.5f * ratioX));
			int y = hotbarStartY - static_cast<int>(s_HotbarSlotSize * ratioY) + ratioY * 6;
			renderer.DrawTexture(x, y, borderTexture, static_cast<int>(borderTexture.GetWidth() * ratioX - ratioX / 2.f), static_cast<int>(borderTexture.GetHeight() * ratioY - ratioX / 2.f), true);
			renderer.DrawTexture(x, y, heartTexture, static_cast<int>(heartTexture.GetWidth() * ratioX - ratioX / 2.f), static_cast<int>(heartTexture.GetHeight() * ratioY - ratioY / 2.f), true);
		}

		// Render armor
		for (int i = 0; i < 20; i += 2)
		{
			int idx = 0;
			if (armor - 1 == i) idx = 1;
			else if (armor < i + 2) idx = 2;

			Texture armorTexture = m_IconsTexture->Crop(16 + idx * 9, 9, 9, 9);

			int x = static_cast<int>(std::floorf(startX + s_HotbarSlotSize * ratioX * 10 - 66 * ratioX + ((i / 2) % 10) * 8.5f * ratioX));
			int y = hotbarStartY - static_cast<int>(s_HotbarSlotSize * ratioY) + ratioY * 6;
			renderer.DrawTexture(x, y, armorTexture, static_cast<int>(armorTexture.GetWidth() * ratioX - ratioX / 2.f), static_cast<int>(armorTexture.GetHeight() * ratioY - ratioY / 2.f), true);
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
			int charWidth = static_cast<int>(fontLoader.GetCharWidth() * ratioX);
			int charHeight = static_cast<int>(fontLoader.GetCharHeight() * ratioY);
			int textX = static_cast<int>(x + (s_SlotSize - 1) * ratioX - charWidth * itemCountStr.size());
			int textY = static_cast<int>(y + s_SlotSize * ratioY - charHeight - ratioY);
			renderer.DrawText(itemCountStr, textX, textY, charWidth, charHeight, fontLoader);
		}
	}
}