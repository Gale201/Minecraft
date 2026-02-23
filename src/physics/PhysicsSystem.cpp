#include "PhysicsSystem.h"
#include "AABB.h"

namespace Minecraft
{
	void PhysicsSystem::Initialize(World& world)
	{
		m_World = &world;
	}

	void PhysicsSystem::Update(float deltaTime)
	{
		for (PhysicsBody* body : m_Bodies)
		{
			if (body->applyGravity)
			{
				body->velocity.y += m_GravityValue * deltaTime;
				body->velocity.y = std::max(body->velocity.y, -m_MaxVelocityValue);
			}

			body->isGrounded = false;
			MoveBody(*body, deltaTime);
		}
	}

	void PhysicsSystem::MoveBody(PhysicsBody& body, float deltaTime)
	{
		if (!body.collidable)
		{
			body.position += body.velocity * deltaTime;
			body.velocity = Vec3f();
			return;
		}

		Vec3f velocity = body.velocity;
		Vec3f displacement = velocity * deltaTime;
		AABB bodyCollider = body.GetAABB();

		Vec3i bodyPos = Vec3i(
			static_cast<int>(std::floorf(body.position.x)),
			static_cast<int>(std::floorf(body.position.y)),
			static_cast<int>(std::floorf(body.position.z))
		);
		std::vector<Vec3i> surroundingBlocks = m_World->GetSurroundingCollidableBlocks(bodyPos);

		// Move on x axis
		for (Vec3i& blockPos : surroundingBlocks)
		{
			AABB blockCollider(blockPos.toVec3f(), Vec3f(1.0f, 1.0f, 1.0f));
			displacement.x = bodyCollider.GetClipX(blockCollider, displacement.x);
		}
		bodyCollider.Move({ displacement.x, 0.0f, 0.0f });

		if (displacement.x != velocity.x * deltaTime) velocity.x = 0;

		// Move on y axis
		for (Vec3i& blockPos : surroundingBlocks)
		{
			AABB blockCollider(blockPos.toVec3f(), Vec3f(1.0f, 1.0f, 1.0f));
			displacement.y = bodyCollider.GetClipY(blockCollider, displacement.y);
		}
		bodyCollider.Move({ 0.0f, displacement.y, 0.0f });

		if (displacement.y != velocity.y * deltaTime)
		{
			velocity.y = 0;
			if (body.velocity.y < 0.0f) body.isGrounded = true;
		}

		// Move on z axis
		for (Vec3i& blockPos : surroundingBlocks)
		{
			AABB blockCollider(blockPos.toVec3f(), Vec3f(1.0f, 1.0f, 1.0f));
			displacement.z = bodyCollider.GetClipZ(blockCollider, displacement.z);
		}
		bodyCollider.Move({ 0.0f, 0.0f, displacement.z });

		if (displacement.z != velocity.z * deltaTime) velocity.z = 0;

		body.position = bodyCollider.GetPosition();
		body.velocity = velocity;
	}

	void PhysicsSystem::AddBody(PhysicsBody& body)
	{
		m_Bodies.push_back(&body);
	}
}