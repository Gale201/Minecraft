#pragma once

#include <assert.h>
#include <array>

#include "../items/ItemStack.h"
#include "../items/ItemRegistry.h"
#include "../render/Renderer.h"
#include "../font/FontLoader.h"
#include "../core/InputManager.h"
#include "../entities/EntityModel.h"

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
		Inventory()
		{
			m_SlotHoverTexture.SetPixel(0, 0, 0x44FFFFFF);
		}

		void Update(const Core::InputManager& inputManager);
		void UpdateHotbar(const Core::InputManager& inputManager);
		void UpdatePlayerModel(const Core::InputManager& inputManager);

		void Render(Renderer& renderer, const ItemRegistry& itemRegistry, const FontLoader& fontLoader);
		void RenderHotbar(Renderer& renderer, const ItemRegistry& itemRegistry, const FontLoader& fontLoader);
		void RenderHotbarIcons(Renderer& renderer, int health, int armor);
		void RenderPlayerModel(Renderer& renderer, float ratioX, float ratioY);

		void GrabItemStack(int selectedSlotX, int selectedSlotY);
		void ReleaseItemStack(int selectedSlotX, int selectedSlotY);
		void ReleaseOneItem(int selectedSlotX, int selectedSlotY);
		void CollectAllItems(int selectedSlotX, int selectedSlotY);

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
		ItemStack GetItemToRemove() const { return m_ItemToRemove; }

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

		void SetTextures(
			Texture& inventroyTexture, 
			Texture& hotbarTexture, 
			Texture& selectedTexture,
			Texture& iconsTexture,
			Texture& steveSkinTexture
		)
		{
			m_InventoryTexture = &inventroyTexture;
			m_HotbarTexture = &hotbarTexture;
			m_SelectedSlotTexture = &selectedTexture;
			m_IconsTexture = &iconsTexture;
			m_SkinTexture = &steveSkinTexture;
		}

		void SetPlayerModel(std::unique_ptr<EntityModel>& playerModel)
		{
			m_PlayerModel = std::make_unique<EntityModel>(*playerModel);
			m_PlayerModel->SetTexture(*m_SkinTexture);
		}

	private:
		void RenderItem(Renderer& renderer, const ItemRegistry& itemRegistry, const FontLoader& fontLoader, const ItemStack& stack, int x, int y, float ratioX, float ratioY);

	private:
		std::array<ItemStack, s_Size> m_Slots;
		std::array<ItemStack, s_CraftAreaSize> m_CraftingSlots;

		Vec2f m_SlotsTopLeftPoint{ 7.f, 83.f };
		Vec2f m_PlayerModelTopLeftPoint{ 25.f, 8.f };

		int m_SelectedHotbarSlotIndex = 0;
		ItemStack m_HoldingStack;
		Vec2i m_HoldingStackPosition;
		bool m_WaitForMouseRelease = false;

		ItemStack m_ItemToRemove;

		Texture* m_InventoryTexture = nullptr;
		Texture* m_HotbarTexture = nullptr;
		Texture* m_SelectedSlotTexture = nullptr;
		Texture* m_IconsTexture = nullptr;
		Texture* m_SkinTexture = nullptr;
		Texture m_SlotHoverTexture{ 1, 1 };

		std::unique_ptr<EntityModel> m_PlayerModel = nullptr;

		int m_RendererWidth = 0;
		int m_RendererHeight = 0;
		int m_MouseX = 0;
		int m_MouseY = 0;
	};
}