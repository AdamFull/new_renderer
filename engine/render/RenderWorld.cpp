#include "RenderWorld.h"

#include "GPUVector.h"

using namespace engine::render;

constexpr const u64 MAX_RENDER_OBJECT_COUNT{ 1024ull };
constexpr const u64 MAT_FRUSTUM_COUNT{ 128ull };
constexpr const u64 MAX_MATERIAL_COUNT{ 1024ull };
constexpr const u64 MAX_INDIRECT_DRAW_COUNT{ 1024ull };

template<class _Ty>
stl::unique_ptr<IGPUVector> createDrawBuffer(const stl::string& name, EBufferTypeFlagBits type, u64 count, IRenderInterface* render_interface)
{
	auto new_buffer = stl::make_unique<CGPUVector>(render_interface);
	new_buffer->create<_Ty>(name, type, count);
	return new_buffer;
}

CRenderWorld::CRenderWorld(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CRenderWorld::create()
{
	m_vBuffers.resize(utl::enum_cast(ERenderBufferIndices::eLastIndex));

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eNodeBuffer)) = createDrawBuffer<FSceneNode>("NodeStorage", EBufferTypeFlagBits::eStorage, MAX_RENDER_OBJECT_COUNT, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eSkinBuffer)) = createDrawBuffer<glm::mat4>("SkinningJoints", EBufferTypeFlagBits::eStorage, MAX_RENDER_OBJECT_COUNT, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eMeshBuffer)) = createDrawBuffer<FMesh>("MeshBuffer", EBufferTypeFlagBits::eStorage, MAX_RENDER_OBJECT_COUNT, m_pRenderInterface);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eMaterialBuffer)) = createDrawBuffer<FMaterial>("MaterialStorage", EBufferTypeFlagBits::eStorage, MAX_MATERIAL_COUNT, m_pRenderInterface);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eDirectionalLightBuffer)) = createDrawBuffer<FDirectionalLightCommit>("DirectionalLights", EBufferTypeFlagBits::eStorage, max_directional_light_count, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::ePointLightBuffer)) = createDrawBuffer<FPointLightCommit>("PointLights", EBufferTypeFlagBits::eStorage, max_point_light_count, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eSpotLightBuffer)) = createDrawBuffer<FSpotLightCommit>("SpotLights", EBufferTypeFlagBits::eStorage, max_spot_light_count, m_pRenderInterface);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eCascadedShadowBuffer)) = createDrawBuffer<FCascadedShadowCommit>("CascadedShadows", EBufferTypeFlagBits::eStorage, max_directional_light_shadow_count, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eOmniShadowBuffer)) = createDrawBuffer<FOmniShadowCommit>("OmniShadows", EBufferTypeFlagBits::eStorage, max_point_light_shadow_count, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eSpotShadowBuffer)) = createDrawBuffer<FSpotShadowCommit>("SpotShadows", EBufferTypeFlagBits::eStorage, max_spot_light_shadow_count, m_pRenderInterface);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eDrawMeshInfoBuffer)) = createDrawBuffer<FDrawMeshinfo>("GeometryBatches", EBufferTypeFlagBits::eStorage, MAX_RENDER_OBJECT_COUNT, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eClusterInfoBuffer)) = createDrawBuffer<FClusterInfo>("ClusterInfoBuffer", EBufferTypeFlagBits::eStorage, MAX_RENDER_OBJECT_COUNT, m_pRenderInterface);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eFrustumBuffer)) = createDrawBuffer<glm::vec4>("FrustumSides", EBufferTypeFlagBits::eStorage, MAT_FRUSTUM_COUNT * 6ull, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eDrawCommandBuffer)) = createDrawBuffer<FDrawIndexedIndirectData>("IndexedDrawIndirectBuffer", EBufferTypeFlagBits::eIndirect, MAX_INDIRECT_DRAW_COUNT, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eMeshCommandBuffer)) = createDrawBuffer<FDrawMeshIndirectData>("MeshIndirectBuffer", EBufferTypeFlagBits::eIndirect, MAX_INDIRECT_DRAW_COUNT, m_pRenderInterface);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eTextureRemapBuffer)) = createDrawBuffer<uint32_t>("TextureBindingRemap", EBufferTypeFlagBits::eStorage, 16384u, m_pRenderInterface);
}

u64 CRenderWorld::registerFrustum(ERenderGeometryCullerType type)
{
	u64 frustum_id = m_vSceneFrustums.size();
	m_vSceneFrustums.emplace_back();

	auto& culling_info = m_vCullingInfo.emplace_back();
	culling_info.cullerID = frustum_id;
	culling_info.type = type;

	return frustum_id;
}

void CRenderWorld::updateFrustum(u64 frustum_id, const glm::mat4& view, const glm::mat4& projection)
{
	auto& frustum = m_vSceneFrustums.at(frustum_id);
	frustum.update(view, projection);
}

void CRenderWorld::addDrawGeometry(const stl::vector<FGeometryRawBatch>& new_geometry)
{
	m_vRawGeometryBatches.insert(m_vRawGeometryBatches.end(), new_geometry.begin(), new_geometry.end());

	// Sort drawings by vbo, material and node
	stl::sort(m_vRawGeometryBatches.begin(), m_vRawGeometryBatches.end(), [](const FGeometryRawBatch& a, const FGeometryRawBatch& b) { return a.drawInfo.vboID < b.drawInfo.vboID; });
}

void CRenderWorld::render(f32 dt)
{
	// Clear old frame data
	m_vGeometryBatches.clear();

	prepareIndirectCommands();
	//prepareMeshletCommands();
	m_vRawGeometryBatches.clear();

	// Copy frustum sides to gpu buffer
	//{
	//	auto& frustum_buffer = getDrawBuffer(ERenderBufferIndices::eFrustumBuffer);
	//	frustum_buffer->map();
	//
	//	u64 offset{ 0ull };
	//	for (const auto& frustum : m_vSceneFrustums)
	//	{
	//		for (const auto& side : frustum.frustumSides)
	//		{
	//			frustum_buffer->write(&side, sizeof(glm::vec4), offset);
	//			offset += sizeof(glm::vec4);
	//		}
	//	}
	//
	//	frustum_buffer->unmap();
	//}

	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& texture_remap_data = render_context->getGlobalTextureRemap();

	auto& texture_remap_buffer = getDrawBuffer(ERenderBufferIndices::eTextureRemapBuffer);
	texture_remap_buffer->clear();
	texture_remap_buffer->copy(texture_remap_data);

	auto& skin_buffer = getDrawBuffer(ERenderBufferIndices::eSkinBuffer);
	skin_buffer->clear();

	auto& mesh_buffer = getDrawBuffer(ERenderBufferIndices::eMeshBuffer);
	mesh_buffer->clear();
}

void CRenderWorld::addLight(const FDirectionalLightCommit& light)
{
	auto& lights_buffer = getDrawBuffer(ERenderBufferIndices::eDirectionalLightBuffer);
	if (lights_buffer->size() < m_stats.directionalLightCount + 1ull)
		lights_buffer->resize((m_stats.directionalLightCount + 1ull) * 2ull);

	lights_buffer->get<FDirectionalLightCommit>(m_stats.directionalLightCount++) = light;
}

void CRenderWorld::addLight(const FPointLightCommit& light)
{
	auto& lights_buffer = getDrawBuffer(ERenderBufferIndices::ePointLightBuffer);

	if (lights_buffer->size() < m_stats.pointLightCount + 1ull)
		lights_buffer->resize((m_stats.pointLightCount + 1ull) * 2ull);

	lights_buffer->get<FPointLightCommit>(m_stats.pointLightCount++) = light;
}

void CRenderWorld::addLight(const FSpotLightCommit& light)
{
	auto& lights_buffer = getDrawBuffer(ERenderBufferIndices::eSpotLightBuffer);

	if (lights_buffer->size() < m_stats.spotLightCount + 1ull)
		lights_buffer->resize((m_stats.spotLightCount + 1ull) * 2ull);

	lights_buffer->get<FSpotLightCommit>(m_stats.spotLightCount++) = light;
}

void CRenderWorld::addShadow(const FCascadedShadowCommit& commit)
{
	auto& shadows_buffer = getDrawBuffer(ERenderBufferIndices::eCascadedShadowBuffer);

	if (shadows_buffer->size() < m_stats.cascadedShadowCount + 1ull)
		shadows_buffer->resize((m_stats.cascadedShadowCount + 1ull) * 2ull);

	shadows_buffer->get<FCascadedShadowCommit>(m_stats.cascadedShadowCount++) = commit;
}

void CRenderWorld::addShadow(const FOmniShadowCommit& commit)
{
	auto& shadows_buffer = getDrawBuffer(ERenderBufferIndices::eOmniShadowBuffer);

	if (shadows_buffer->size() < m_stats.omniShadowCount + 1ull)
		shadows_buffer->resize((m_stats.omniShadowCount + 1ull) * 2ull);

	shadows_buffer->get<FOmniShadowCommit>(m_stats.omniShadowCount++) = commit;
}

void CRenderWorld::addShadow(const FSpotShadowCommit& commit)
{
	auto& shadows_buffer = getDrawBuffer(ERenderBufferIndices::eSpotShadowBuffer);

	if (shadows_buffer->size() < m_stats.spotShadowCount + 1ull)
		shadows_buffer->resize((m_stats.spotShadowCount + 1ull) * 2ull);

	shadows_buffer->get<FSpotShadowCommit>(m_stats.spotShadowCount++) = commit;
}

void CRenderWorld::updateNode(u32 node_id, const FSceneNode& node)
{
	auto& nodes_buffer = getDrawBuffer(ERenderBufferIndices::eNodeBuffer);

	if (nodes_buffer->size() < node_id + 1ull)
		nodes_buffer->resize((node_id + 1ull));

	nodes_buffer->get<FSceneNode>(node_id) = node;
}

void CRenderWorld::updateSkin(const stl::vector<glm::mat4>& matrices)
{
	auto& skin_buffer = getDrawBuffer(ERenderBufferIndices::eSkinBuffer);
	skin_buffer->copy(matrices);
	m_stats.skinCount += matrices.size();
}

void CRenderWorld::addMaterial(const FMaterial& material)
{
	auto& materials_buffer = getDrawBuffer(ERenderBufferIndices::eMaterialBuffer);
	materials_buffer->emplace_back(material);
}

void CRenderWorld::addMaterials(const stl::vector<FMaterial>& materials)
{
	auto& materials_buffer = getDrawBuffer(ERenderBufferIndices::eMaterialBuffer);

	for (auto& material : materials)
		materials_buffer->emplace_back(material);
}

void CRenderWorld::addMesh(const FMesh& mesh)
{
	auto& mesh_buffer = getDrawBuffer(ERenderBufferIndices::eMeshBuffer);
	mesh_buffer->emplace_back(mesh);
	m_stats.meshCount++;
}

void CRenderWorld::addMeshes(const std::vector<FMesh>& meshes)
{
	auto& mesh_buffer = getDrawBuffer(ERenderBufferIndices::eMeshBuffer);
	mesh_buffer->copy(meshes);
	m_stats.meshCount += meshes.size();
}

const stl::unique_ptr<IGPUVector>& CRenderWorld::getDrawBuffer(ERenderBufferIndices buffer_index)
{
	return m_vBuffers.at(utl::enum_cast(buffer_index));
}

const stl::vector<FGeometryCPUBatch>& CRenderWorld::getGeometryBatches() const
{
	return m_vGeometryBatches;
}

FRenderWorldStats& CRenderWorld::getStats()
{
	return m_stats;
}

const FRenderWorldStats& CRenderWorld::getStats() const
{
	return m_stats;
}

void CRenderWorld::prepareIndirectCommands()
{
	auto& indirect_data_buffer = getDrawBuffer(ERenderBufferIndices::eDrawCommandBuffer);
	indirect_data_buffer->clear();

	auto& gpu_batches_buffer = getDrawBuffer(ERenderBufferIndices::eDrawMeshInfoBuffer);
	gpu_batches_buffer->clear();

	std::vector<FDrawIndexedIndirectData> indirect_data{};
	std::vector<FDrawMeshinfo> gpu_batches{};

	stl::unordered_map<u64, u64> indirect_command_cache{};

	for (const auto& batch : m_vRawGeometryBatches)
	{
		// Create batch or get already existing
		FGeometryCPUBatch* current_batch{ nullptr };
		if (!m_vGeometryBatches.empty() && m_vGeometryBatches.back().vboID == batch.drawInfo.vboID)
			current_batch = &m_vGeometryBatches.back();
		else
		{
			auto& new_batch = m_vGeometryBatches.emplace_back();
			new_batch.vboID = batch.drawInfo.vboID;
			new_batch.command.bufferId = indirect_data_buffer->buffer()->handle();
			new_batch.command.offset = indirect_data.size() * sizeof(FDrawIndexedIndirectData);
			new_batch.command.count = 0;
			new_batch.command.stride = sizeof(FDrawIndexedIndirectData);

			current_batch = &new_batch;
		}

		// Check for range is already exists in command list
		u64 range_hash = utl::utils::murmur_hash_64(&batch.range, sizeof(FGeometryDrawRange), 0ull);
		if (auto found = indirect_command_cache.find(range_hash); found != indirect_command_cache.end())
		{
			auto& indirect_command = indirect_data.at(found->second);
			indirect_command.instanceCount++;
		}
		else
		{
			// Cache current command
			indirect_command_cache.emplace(range_hash, indirect_data.size());

			auto& vbo = m_pRenderInterface->getVertexHandler(batch.drawInfo.vboID);

			auto last_index = vbo->getLastIndex();
			auto last_vertex = vbo->getLastVertex();

			// Create new draw command
			auto& indirect_command = indirect_data.emplace_back();
			indirect_command.indexCount = batch.range.indexCount == 0ull ? last_index : batch.range.indexCount;;
			indirect_command.instanceCount = 1u;
			indirect_command.firstIndex = batch.range.beginIndex;
			indirect_command.vertexOffset = batch.range.beginVertex;
			indirect_command.firstInstance = 0u;
			batch.range.vertexCount == 0ull ? last_vertex : batch.range.vertexCount;

			current_batch->command.count++;
		}

		// Save draw order in all draw array
		gpu_batches.emplace_back(batch.drawInfo);
	}

	m_stats.drawCommandCount = indirect_data.size();

	indirect_data_buffer->copy(indirect_data);
	gpu_batches_buffer->copy(gpu_batches);
}

void CRenderWorld::prepareMeshletCommands()
{
	auto& indirect_data_buffer = getDrawBuffer(ERenderBufferIndices::eMeshCommandBuffer);
	indirect_data_buffer->clear();

	auto& cluster_info_buffer = getDrawBuffer(ERenderBufferIndices::eClusterInfoBuffer);
	cluster_info_buffer->clear();

	auto& gpu_batches_buffer = getDrawBuffer(ERenderBufferIndices::eDrawMeshInfoBuffer);
	gpu_batches_buffer->clear();

	std::vector<FDrawMeshIndirectData> indirect_data{};
	std::vector<FDrawMeshinfo> gpu_batches{};
	std::vector<FClusterInfo> gpu_clusters{};

	stl::unordered_map<u64, u64> indirect_command_cache{};

	for (const auto& batch : m_vRawGeometryBatches)
	{
		FGeometryCPUBatch* current_batch{ nullptr };
		if (!m_vGeometryBatches.empty() && m_vGeometryBatches.back().vboID == batch.drawInfo.vboID)
			current_batch = &m_vGeometryBatches.back();
		else
		{
			auto& new_batch = m_vGeometryBatches.emplace_back();
			new_batch.vboID = batch.drawInfo.vboID;
			new_batch.command.bufferId = indirect_data_buffer->buffer()->handle();
			new_batch.command.offset = indirect_data.size() * sizeof(FDrawMeshIndirectData);
			new_batch.command.count = 0;
			new_batch.command.stride = sizeof(FDrawMeshIndirectData);

			current_batch = &new_batch;
		}

		// Create draw call data
		for (u64 cidx = 0ull; cidx < batch.range.meshletCount; ++cidx)
		{
			auto& cluster_info = gpu_clusters.emplace_back();
			cluster_info.drawID = gpu_batches.size();
			cluster_info.firstMeshlet = batch.range.firstMeshlet;
			cluster_info.meshletCount = batch.range.meshletCount;
		}

		//u64 range_hash = utl::utils::murmur_hash_64(&batch.range, sizeof(FGeometryDrawRange), 0ull);
		//if (auto found = indirect_command_cache.find(range_hash); found != indirect_command_cache.end())
		//{
		//	auto& indirect_command = indirect_data.at(found->second);
		//	indirect_command.instanceCount++;
		//}
		//else
		//{
			// Cache current command
		//	indirect_command_cache.emplace(range_hash, indirect_data.size());

			auto& vbo = m_pRenderInterface->getVertexHandler(batch.drawInfo.vboID);

			auto last_index = vbo->getLastIndex();
			auto last_vertex = vbo->getLastVertex();

			// Create new draw command
			auto& indirect_command = indirect_data.emplace_back();
			indirect_command.firstTask = 0;
			indirect_command.taskCount = (batch.range.meshletCount + kShaderGroupSize - 1) / kShaderGroupSize;

			current_batch->command.count++;
		//}

		// Save draw order in all draw array
		gpu_batches.emplace_back(batch.drawInfo);
	}

	m_stats.drawCommandCount = indirect_data.size();

	indirect_data_buffer->copy(indirect_data);
	cluster_info_buffer->copy(gpu_clusters);
	gpu_batches_buffer->copy(gpu_batches);
}