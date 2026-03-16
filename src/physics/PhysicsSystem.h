#pragma once
#include <array>
#include "PhysicsBody.h"
#include "../world/World.h"
#include "../thread/ThreadSystem.h"

namespace Minecraft
{
	class PhysicsSystem
	{
	public:
		static const int MAX_PHYSICS_BODIES = 10000;

	public:
		PhysicsSystem(Core::ThreadSystem& threadSystem) :
			m_ThreadSystem(threadSystem)
		{
			for (size_t i = 0; i < MAX_PHYSICS_BODIES; i++)
				m_FreeSlots.push_back(i);
		};

		void Initialize(World& world);

		void Update(float deltaTime);

		PhysicsBody* AddBody(PhysicsBody body);
		void RemoveBody(PhysicsBody* body);

	private:
		void ApplyGravity(PhysicsBody& body, float deltaTime);
		void CalculateVelocity(PhysicsBody& body, float deltaTime);
		void MoveBody(PhysicsBody& body, float deltaTime);

	private:
		World* m_World = nullptr;
		std::array<PhysicsBody, MAX_PHYSICS_BODIES> m_Bodies;
		std::vector<size_t> m_FreeSlots;
		std::vector<size_t> m_ActiveBodies;
		
		Core::ThreadSystem& m_ThreadSystem;

		float m_GravityValue = -30.f;
		float m_MaxVelocityValue = 500.0f;
	};
}
