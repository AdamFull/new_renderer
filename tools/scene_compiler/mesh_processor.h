#pragma once

#include <easygltf/easygltf_base.h>
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

	class CGLTFEditor
	{
	public:
		CGLTFEditor();

		bool load(const std::string& path);
		bool save(const std::string& path);

		void process();
	protected:
		void prepare();
		void apply();

		void mergeMaterials();
		void filterEmptyMeshes(int32_t mesh_id, std::vector<FMesh>& meshes);
		void reindexMesh(FMesh& mesh);
		void filterTriangles(FMesh& mesh);
		void simplifyMesh(FMesh& mesh, float threshold = 1.f, bool agressive = false, bool lock_borders = false);
		void optimizeMesh(FMesh& mesh, bool compressmore = false);

		bool checkMergeAbility(int32_t mesh_id, FMesh& lhs, FMesh& rhs);
		void mergeMeshes(FMesh& target, const FMesh& mesh);
		void mergeMeshes(int32_t mesh_id, std::vector<FMesh>& meshes);
	private:
		easygltf::FGLTFRoot m_root{};

		std::unordered_map<int32_t, std::vector<FMesh>> m_meshes{};
		std::unordered_map<int32_t, FSkin> m_skins{};
		std::unordered_map<int32_t, std::vector<FAnimationSampler>> m_samplers{};
	};
}