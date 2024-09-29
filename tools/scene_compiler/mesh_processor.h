#pragma once

#include <easygltf/easygltf_base.h>
#include <vector>

namespace mesh_compiler
{
	struct FStream
	{
		uint8_t* get(size_t index = 0ull) { return &data[index * stride]; }

		size_t stride{ 0ull };
		size_t count{ 0ull };
		utl::vector<uint8_t> data{};
	};

	struct FVertexStream
	{
		uint32_t type{};
		FStream buffer{};
	};

	struct FMesh
	{
		utl::vector<FVertexStream> streams{};
		utl::vector<utl::vector<FVertexStream>> morphs{};
		utl::vector<uint32_t> indices{};
	};

	struct FSkin
	{
		FStream buffer{};
	};

	struct FAnimationSampler
	{
		FStream input{};
		FStream output{};
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
		void filterEmptyMeshes(int32_t mesh_id, utl::vector<FMesh>& meshes);
		void reindexMesh(FMesh& mesh);
		void filterTriangles(FMesh& mesh);
		void simplifyMesh(FMesh& mesh, float threshold = 1.f, bool agressive = false, bool lock_borders = false);
		void optimizeMesh(FMesh& mesh, bool compressmore = false);

		bool checkMergeAbility(int32_t mesh_id, FMesh& lhs, FMesh& rhs);
		void mergeMeshes(FMesh& target, const FMesh& mesh);
		void mergeMeshes(int32_t mesh_id, utl::vector<FMesh>& meshes);
	private:
		easygltf::FGLTFRoot m_root{};

		std::unordered_map<int32_t, utl::vector<FMesh>> m_meshes{};
		std::unordered_map<int32_t, FSkin> m_skins{};
		std::unordered_map<int32_t, utl::vector<FAnimationSampler>> m_samplers{};
	};
}