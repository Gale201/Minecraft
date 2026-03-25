#include "Mesh.h"

namespace Minecraft
{
	Mesh CreateBlockMesh(BlockType blockType, float scale, Vec3f centerOffset)
	{
		Mesh mesh;
		for (int dir = 0; dir < 6; dir++)
		{
			int startIndex = static_cast<int>(mesh.vertices.size());

			FaceDirection faceDir = static_cast<FaceDirection>(dir);
			BlockFace face = GetBlockFace(blockType, faceDir);

			for (int i = 0; i < 4; i++)
			{
				Vertex v;
				v.position = Vec3f(
					face.vertices[i].x + centerOffset.x,
					face.vertices[i].y + centerOffset.y,
					face.vertices[i].z + centerOffset.z
				) * scale;
				v.normal = face.normal;

				v.u = face.uvs[i].x;
				v.v = face.uvs[i].y;

				TileType tileType = GetTileTypeFromDirection(blockType, faceDir);
				v.tileInfo = GetAtlasTileInfo(tileType);

				mesh.vertices.push_back(v);
			}

			mesh.indices.push_back(startIndex + 0);
			mesh.indices.push_back(startIndex + 1);
			mesh.indices.push_back(startIndex + 2);

			mesh.indices.push_back(startIndex + 0);
			mesh.indices.push_back(startIndex + 2);
			mesh.indices.push_back(startIndex + 3);
		}
		return mesh;
	}

	Mesh CreateBlockMesh(AtlasTileInfo tileInfo, float scale, Vec3f centerOffset)
	{
		Mesh mesh;
		for (int dir = 0; dir < 6; dir++)
		{
			int startIndex = static_cast<int>(mesh.vertices.size());

			FaceDirection faceDir = static_cast<FaceDirection>(dir);
			BlockFace face = GetBlockFace(BlockType::Dirt, faceDir);

			for (int i = 0; i < 4; i++)
			{
				Vertex v;
				v.position = Vec3f(
					face.vertices[i].x + centerOffset.x,
					face.vertices[i].y + centerOffset.y,
					face.vertices[i].z + centerOffset.z
				) * scale;
				v.normal = face.normal;

				v.u = face.uvs[i].x;
				v.v = face.uvs[i].y;

				v.tileInfo = tileInfo;

				mesh.vertices.push_back(v);
			}

			mesh.indices.push_back(startIndex + 0);
			mesh.indices.push_back(startIndex + 1);
			mesh.indices.push_back(startIndex + 2);

			mesh.indices.push_back(startIndex + 0);
			mesh.indices.push_back(startIndex + 2);
			mesh.indices.push_back(startIndex + 3);
		}
		return mesh;
	}
}