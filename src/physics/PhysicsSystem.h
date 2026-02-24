#pragma once
#include <vector>
#include "PhysicsBody.h"
#include "../world/World.h"

namespace Minecraft
{
	class PhysicsSystem
	{
	public:
		PhysicsSystem() = default;

		void Initialize(World& world);

		void Update(float deltaTime);
		void ApplyGravity(PhysicsBody& body, float deltaTime);
		void CalculateVelocity(PhysicsBody& body, float deltaTime);
		void MoveBody(PhysicsBody& body, float deltaTime);

		void AddBody(PhysicsBody& body);

	private:
		World* m_World = nullptr;
		std::vector<PhysicsBody*> m_Bodies;

		float m_GravityValue = -30.f;
		float m_MaxVelocityValue = 500.0f;
	};
}
