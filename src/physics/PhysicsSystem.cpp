#include "PhysicsSystem.h"
#include "AABB.h"
#include "../timer/Timer.h"

namespace Minecraft
{
	void PhysicsSystem::Initialize(World& world)
	{
		m_World = &world;
	}

	void PhysicsSystem::Update(float deltaTime)
	{
		/*m_ThreadSystem.ParallelFor(
			(int)m_Bodies.size(),
			[this, deltaTime](int i)
			{
				PhysicsBody& body = *m_Bodies[i];

				ApplyGravity(body, deltaTime);
				
				CalculateVelocity(body, deltaTime);
				
				body.isGrounded = false;
				body.inWater = false;
				
				MoveBody(body, deltaTime);
				
				body.ClearTotalForce();
			}
			);*/

		for (PhysicsBody* body : m_Bodies)
		{
			if (body->pausePhysics) continue;

			ApplyGravity(*body, deltaTime);

			CalculateVelocity(*body, deltaTime);

			body->isGrounded = false;
			body->inWater = false;
			body->UpdatePreviousPosition();
			MoveBody(*body, deltaTime);

			body->ClearTotalForce();
		}
	}

	void PhysicsSystem::ApplyGravity(PhysicsBody& body, float deltaTime)
	{
		float gravity = m_GravityValue * body.mass;
		if (body.inWater)
			gravity *= 0.3f;

		if (body.applyGravity)
			body.AddForce({ 0.0f, gravity, 0.0f });
	}

	void PhysicsSystem::CalculateVelocity(PhysicsBody& body, float deltaTime)
	{
		Vec3f acceleration = body.GetTotalForce() * (1.0f / body.mass);
		body.velocity += acceleration * deltaTime;

		if (!body.inWater)
		{
			// Ground friction
			Vec2f horizontalVel = { body.velocity.x, body.velocity.z };
			float friction = body.isGrounded ? body.groundFriction : body.airFriction;
			horizontalVel -= horizontalVel * friction * deltaTime;

			// Air friction
			float verticalVel = body.velocity.y;
			friction = body.isGrounded ? 0.0f : body.flyingFriction;
			verticalVel -= verticalVel * friction * deltaTime;

			body.velocity.x = horizontalVel.x;
			body.velocity.y = verticalVel;
			body.velocity.z = horizontalVel.y;
		}
		// Water drag & buoyancy
		else if (body.inWater)
		{
			body.velocity -= body.velocity * body.waterDrag * deltaTime;
			body.velocity.y += 3.0f * deltaTime;
		}

		body.velocity.x = std::min(std::max(body.velocity.x, -m_MaxVelocityValue), m_MaxVelocityValue);
		body.velocity.y = std::min(std::max(body.velocity.y, - m_MaxVelocityValue), m_MaxVelocityValue);
		body.velocity.z = std::min(std::max(body.velocity.z, -m_MaxVelocityValue), m_MaxVelocityValue);
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
		
		Vec3i bodyPos = Vec3i::floor(body.position);
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

		BlockType type = m_World->GetBlock(Vec3i::floor(body.position)).type;
		if (type == BlockType::Water) body.inWater = true;
	}

	void PhysicsSystem::AddBody(PhysicsBody& body)
	{
		m_Bodies.push_back(&body);
	}
}