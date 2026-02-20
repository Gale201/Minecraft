#pragma once

#include <string>
#include <optional>

#include "../render/Mesh.h"
#include "../render/Model.h"

namespace Minecraft
{
	struct IndexData
	{
		int vertexIndex;
		int uvIndex;
		int normalIndex;
	};

	class ModelLoader
	{
	public:

		ModelLoader(std::string objFolderPath);
		~ModelLoader() = default;

		std::optional<Model> LoadModelFromObj(const std::string fileName);

		std::string GetModelFolderPath() const { return m_ModelFolderPath; }

	private:
		std::string m_ModelFolderPath;

		void CombineParsedDataForMesh(
			Mesh& mesh,
			std::vector<IndexData>& indices,
			std::vector<Vec3f>& positions,
			std::vector<Vec3f> normals,
			std::vector<Vec2f> uvs
		);
	};
}
