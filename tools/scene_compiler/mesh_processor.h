#pragma once

#include <easygltf/easygltf_fwd.h>

namespace mesh_compiler
{
	struct FVertexStream
	{
		uint32_t type{};

		std::vector<uint8_t> data{};

		size_t stride{ 0ull };
		size_t count{ 0ull };
	};

	struct FMesh
	{
		int32_t meshIndex{ -1 };

		std::vector<FVertexStream> streams{};
		std::vector<uint32_t> indices{};
	};

	class CMeshPorcessor
	{
	public:
		CMeshPorcessor();

		void prepare(easygltf::CGLTFProject* project);
		void process();
	private:
		std::vector<FMesh> m_meshes{};
	};
}