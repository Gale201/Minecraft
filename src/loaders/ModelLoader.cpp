#define _CRT_SECURE_NO_WARNINGS

#include "ModelLoader.h"

#include <fstream>
#include <vector>

#include "../core/Log.h"

namespace Minecraft
{
	ModelLoader::ModelLoader(std::string modelFolderPath)
		: m_ModelFolderPath(modelFolderPath)
	{
		if (m_ModelFolderPath.back() != '/' && m_ModelFolderPath.back() != '\\')
		{
			m_ModelFolderPath += '/';
		}
	}

	std::optional<Model> ModelLoader::LoadModelFromObj(const std::string fileName) const
	{
		// Open file and check if it opened successfully
		std::ifstream file(m_ModelFolderPath + fileName);
		if (!file.is_open())
			return {};

		// Temporary storage for positions, uvs, normals, and indices
		std::vector<Vec3f> positions;
		std::vector<IndexData> indices;
		std::vector<Vec2f> uvs;
		std::vector<Vec3f> normals;
		Vec3f pivot;

		std::vector<ModelPart> modelParts;

		int lastMeshStartIndex = 0;

		std::string line;
		while (std::getline(file, line))
		{
			// Temporary variables for parsing
			Vec3f position, normal;
			Vec2f uv;
			int vertexIndex[4], uvIndex[4], normalIndex[4];

			// Parsing vertex lines
			if (
				line.starts_with("v ") &&
				std::sscanf(line.c_str(), "v %f %f %f", &position.x, &position.y, &position.z) == 3
				)
			{
				positions.push_back(position);
			}
			// Parsin face lines for quads
			else if (
				line.starts_with("f ") &&
				std::sscanf(
					line.c_str(),
					"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
					&vertexIndex[0], &uvIndex[0], &normalIndex[0],
					&vertexIndex[1], &uvIndex[1], &normalIndex[1],
					&vertexIndex[2], &uvIndex[2], &normalIndex[2],
					&vertexIndex[3], &uvIndex[3], &normalIndex[3]
				) == 12
				)
			{
				indices.push_back({ vertexIndex[0], uvIndex[0], normalIndex[0] });
				indices.push_back({ vertexIndex[1], uvIndex[1], normalIndex[1] });
				indices.push_back({ vertexIndex[2], uvIndex[2], normalIndex[2] });

				indices.push_back({ vertexIndex[0], uvIndex[0], normalIndex[0] });
				indices.push_back({ vertexIndex[2], uvIndex[2], normalIndex[2] });
				indices.push_back({ vertexIndex[3], uvIndex[3], normalIndex[3] });
			}
			// Parsing face lines for triangles
			else if (
				line.starts_with("f ") &&
				std::sscanf(
					line.c_str(),
					"f %d/%d/%d %d/%d/%d %d/%d/%d",
					&vertexIndex[0], &uvIndex[0], &normalIndex[0],
					&vertexIndex[1], &uvIndex[1], &normalIndex[1],
					&vertexIndex[2], &uvIndex[2], &normalIndex[2]
				) == 9
				)
			{
				indices.push_back({ vertexIndex[0], uvIndex[0], normalIndex[0] });
				indices.push_back({ vertexIndex[1], uvIndex[1], normalIndex[1] });
				indices.push_back({ vertexIndex[2], uvIndex[2], normalIndex[2] });
			}
			// Parsing texture coordinate lines
			else if (
				line.starts_with("vt ") &&
				std::sscanf(line.c_str(), "vt %f %f", &uv.x, &uv.y) == 2
				)
			{
				uvs.push_back(uv);
			}
			// Parsing normal lines
			else if (
				line.starts_with("vn ") &&
				std::sscanf(line.c_str(), "vn %f %f %f", &normal.x, &normal.y, &normal.z) == 3
				)
			{
				normals.push_back(normal);
			}
			// Parsing pivots
			else if (
				line.starts_with("pivot ") &&
				std::sscanf(line.c_str(), "pivot %f %f %f", &pivot.x, &pivot.y, &pivot.z) == 3
				)
			{
				if (!modelParts.empty())
				{
					modelParts.back().pivot = pivot;
				}
			}
			// Handle object/group lines
			else if (line.starts_with("o ") || line.starts_with("g "))
			{
				if (!modelParts.empty())
				{
					CombineParsedDataForMesh(
						modelParts.back().mesh,
						indices,
						positions,
						normals,
						uvs,
						modelParts.back().pivot,
						lastMeshStartIndex
					);

					lastMeshStartIndex = indices.size();
				}
				modelParts.emplace_back();
				modelParts.back().name = line.substr(2, line.size() - 2);
			}
		}

		CombineParsedDataForMesh(
			modelParts.back().mesh,
			indices,
			positions,
			normals,
			uvs,
			modelParts.back().pivot,
			lastMeshStartIndex
		);

		Model model;
		model.parts = modelParts;

		return model;
	}

	void ModelLoader::CombineParsedDataForMesh(
		Mesh& mesh,
		std::vector<IndexData>& indices,
		std::vector<Vec3f>& positions,
		std::vector<Vec3f>& normals,
		std::vector<Vec2f>& uvs,
		Vec3f pivot,
		int lastMeshStartIndex
	) const
	{
		std::vector<Vertex> vertices;
		std::vector<int> combinedIndices;

		// Combine the parsed data into vertices and indices
		for (int i = lastMeshStartIndex; i < indices.size(); i++)
		{
			IndexData indexData = indices[i];
			Vertex vertex;

			vertex.position = positions[indexData.vertexIndex - 1] - pivot;
			vertex.u = uvs[indexData.uvIndex - 1].x;
			vertex.v = 1.0f - uvs[indexData.uvIndex - 1].y;
			vertex.normal = normals[indexData.normalIndex - 1];

			vertices.push_back(vertex);
			combinedIndices.push_back(i - lastMeshStartIndex);
		}

		mesh.vertices = vertices;
		mesh.indices = combinedIndices;
	}
}
