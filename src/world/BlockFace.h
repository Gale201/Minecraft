#pragma once

#include <cinttypes>

#include "Block.h"
#include "../math/Math.h"

enum class FaceDirection : int
{
	PosX = 0,
	NegX,
	PosY,
	NegY,
	PosZ,
	NegZ
};

struct BlockFace
{
	Vec3f vertices[4];
	Vec3f normal;
	Vec2f uvs[4];
};

struct BlockTypeInfo
{
	Vec2f uvOffset[6];
	Vec2f uvScale;
};

void GetFaceUV(BlockType type, FaceDirection dir, Vec2f& uvScale, Vec2f& uvOffset);

extern const FaceDirection FACE_DIRECTIONS[6];

extern const BlockFace CUBE_FACES[6];

extern const Vec3i CUBE_NEIGHBOR_OFFSET[6];

