#pragma once

#include <easygltf/easygltf_fwd.h>
#include <common.h>

namespace mesh_compiler
{
	struct FVertexStream
	{
		uint32_t type{};
		FBuffer buffer{};
	};

	struct FMesh
	{
		std::vector<FVertexStream> streams{};
		std::vector<std::vector<FVertexStream>> morphs{};
		std::vector<uint32_t> indices{};
	};

	struct FSkin
	{
		FBuffer buffer{};
	};

	struct FAnimationSampler
	{
		FBuffer input{};
		FBuffer output{};
	};

	class CMeshPorcessor
	{
	public:
		CMeshPorcessor();

		void prepare(easygltf::CGLTFProject* project);
		void process();
		void apply(easygltf::CGLTFProject* project);
	private:
		std::unordered_map<int32_t, std::vector<FMesh>> m_meshes{};
		std::unordered_map<int32_t, FSkin> m_skins{};
		std::unordered_map<int32_t, std::vector<FAnimationSampler>> m_samplers{};
	};
}