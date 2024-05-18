#include "RenderWorld.h"

#include "GPUVector.h"

using namespace engine::render;

constexpr const u64 MAX_RENDER_OBJECT_COUNT{ 1024ull };
constexpr const u64 MAT_FRUSTUM_COUNT{ 128ull };
constexpr const u64 MAX_MATERIAL_COUNT{ 1024ull };
constexpr const u64 MAX_INDIRECT_DRAW_COUNT{ 1024ull };

namespace mat
{
	template<class _Ty>
	bool isnan(const glm::mat<4, 4, _Ty, glm::defaultp>& mat)
	{
		bool res{ false };
		for (u64 i = 0ull; i < 4; ++i)
			for (u64 j = 0ull; j < 4; ++j)
				res = res || glm::isnan(mat[i][j]);

		return res;
	}
}

CRenderWorld::CRenderWorld(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
	m_vBuffers.resize(utl::enum_cast(ERenderBufferIndices::eLastIndex));

	for (auto& buffer : m_vBuffers)
		buffer = std::make_unique<CGPUVector>(render_interface);
}

void CRenderWorld::create()
{
	m_vBuffers.resize(utl::enum_cast(ERenderBufferIndices::eLastIndex));

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eNodeBuffer))->create<FSceneNode>("NodeStorage", 16ull, EBufferUsageFlagBits::eStorageBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eMatricesBuffer))->create<glm::mat4>("MatricesBuffer", 16ull, EBufferUsageFlagBits::eStorageBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eMeshBuffer))->create<FMesh>("MeshBuffer", 16ull, EBufferUsageFlagBits::eStorageBuffer);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eMaterialBuffer))->create<FMaterial>("MaterialStorage", 16ull, EBufferUsageFlagBits::eStorageBuffer);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eLightBuffer))->create<FLightCommit>("LightsBuffer", max_directional_light_count, EBufferUsageFlagBits::eStorageBuffer);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eCascadedShadowBuffer))->create<FCascadedShadowCommit>("CascadedShadows", max_directional_light_shadow_count, EBufferUsageFlagBits::eStorageBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eOmniShadowBuffer))->create<FOmniShadowCommit>("OmniShadows", max_point_light_shadow_count, EBufferUsageFlagBits::eStorageBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eSpotShadowBuffer))->create<FSpotShadowCommit>("SpotShadows", max_spot_light_shadow_count, EBufferUsageFlagBits::eStorageBuffer);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eDrawMeshInfoBuffer))->create<FGeometryBatch>("GeometryBatches", 16ull, EBufferUsageFlagBits::eStorageBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eRawDrawMeshInfoBuffer))->create<FGeometryBatch>("RawGeometryBatches", 16ull, EBufferUsageFlagBits::eStorageBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eClusterInfoBuffer))->create<FClusterInfo>("ClusterInfoBuffer", 16ull, EBufferUsageFlagBits::eStorageBuffer);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eFrustumBuffer))->create<glm::vec4>("FrustumSides", 16ull, EBufferUsageFlagBits::eStorageBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eDrawCommandBuffer))->create<FDrawIndexedIndirectData>("IndexedDrawIndirectBuffer", 16ull, EBufferUsageFlagBits::eIndirectBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eCountBuffer))->create<FDrawIndexedIndirectData>("IndirectCountBuffer", 16ull, EBufferUsageFlagBits::eIndirectBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eMeshCommandBuffer))->create<FDrawMeshIndirectData>("MeshIndirectBuffer", 16ull, EBufferUsageFlagBits::eIndirectBuffer);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eTextureRemapBuffer))->create<uint32_t>("TextureBindingRemap", 16ull, EBufferUsageFlagBits::eStorageBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eCameraBuffer))->create<FCamera>("CameraBuffer", 16ull, EBufferUsageFlagBits::eStorageBuffer);
}

u64 CRenderWorld::registerCamera(ERenderGeometryCullerType type)
{
	u64 camera_id = m_vSceneFrustums.size();
	m_vSceneFrustums.emplace_back();

	auto& camera_buffer = getDrawBuffer(ERenderBufferIndices::eCameraBuffer);
	auto& camera = *(camera_buffer->emplace_back<FCamera>());
	camera.type = type == ERenderGeometryCullerType::eCascadedShadow;

	auto& culling_info = m_vCullingInfo.emplace_back();
	culling_info.cullerID = camera_id;
	culling_info.type = type;

	return camera_id;
}

void CRenderWorld::updateCamera(u64 camera_id, const glm::mat4& view, const glm::mat4& prev_view, const glm::mat4& projection, const glm::mat4& prev_projection, const glm::vec2& size, float znear, float zfar)
{
	auto& frustum = m_vSceneFrustums.at(camera_id);
	frustum.update(view, projection);

	std::array<glm::mat4, matrix_offset_count> matrices{};
	matrices[view_matrix_offset] = view;																							// view_matrix_offset
	matrices[inv_view_matrix_offset] = glm::inverse(matrices[view_matrix_offset]);													// inv_view_matrix_offset
	matrices[projection_matrix_offset] = projection;																				// projection_matrix_offset
	matrices[inv_projection_matrix_offset] = glm::inverse(matrices[projection_matrix_offset]);										// inv_projection_matrix_offset
	matrices[view_projection_matrix_offset] = matrices[projection_matrix_offset] * matrices[view_matrix_offset];					// view_projection_matrix_offset
	matrices[prev_view_projection_matrix_offset] = prev_projection * prev_view;		// prev_view_projection_matrix_offset
	matrices[inv_view_projection_matrix_offset] = glm::inverse(matrices[view_projection_matrix_offset]);							// inv_view_projection_matrix_offset
	auto& camera_reprojection = matrices[reprojection_matrix_offset];																// reprojection_matrix_offset

	camera_reprojection = matrices[prev_view_projection_matrix_offset] *  matrices[inv_view_projection_matrix_offset];
	//camera_reprojection = matrices[inv_view_projection_matrix_offset] * matrices[prev_view_projection_matrix_offset];

	float rcp_half_dimx = 2.0f / size.x;
	float rcp_half_dimy = 2.0f / size.y;
	float rcp_z_magic = znear / (zfar - znear);

	bool linear_z = false;
	
	// Transform from uv to clip space: texcoord * 2 - 1
	static glm::mat4 to_clip_space =
	{
		rcp_half_dimx, 0.0f, 0.0f, 0.0f,
		0.0f, -rcp_half_dimy, 0.0f, 0.0f,
		0.0f, 0.0f, linear_z ? rcp_z_magic : 1.0f, 0.0f,
		- 1.0f, 1.0f, linear_z ? -rcp_z_magic : 0.0f, 1.0f
	};
	// Transform from clip to uv space: texcoord * 0.5 + 0.5
	static glm::mat4 to_uv_space =
	{
		1.0f / rcp_half_dimx, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f / rcp_half_dimy, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		1.0f / rcp_half_dimx, 1.0f / rcp_half_dimy, 0.0f, 1.0f
	};
	camera_reprojection = to_clip_space * camera_reprojection * to_uv_space;

	auto& camera_buffer = getDrawBuffer(ERenderBufferIndices::eCameraBuffer);
	const FCamera* camera = camera_buffer->at<FCamera>(camera_id);
	camera_buffer->set(camera_id, FCamera{ utl::cast<u32>(m_stats.matricesCount), camera->type, znear, zfar, size, 1.f / size });

	auto& matrices_buffer = getDrawBuffer(ERenderBufferIndices::eMatricesBuffer);
	matrices_buffer->copy(matrices);
	m_stats.matricesCount += matrices.size();
}

const FCamera& CRenderWorld::getCamera(u64 index) const
{
	auto& camera_buffer = getDrawBuffer(ERenderBufferIndices::eCameraBuffer);
	return *(camera_buffer->at<FCamera>(index));
}

const FCamera& CRenderWorld::getDefaultCamera() const
{
	return getCamera(m_defaultCameraID);
}

const u64 CRenderWorld::getCameraID(ERenderGeometryCullerType type) const
{
	for (const auto& camera_info : m_vCullingInfo)
	{
		if (camera_info.type == type)
			return camera_info.cullerID;
	}

	return m_defaultCameraID;
}

const u64 CRenderWorld::getDefaultCameraID() const
{
	return m_defaultCameraID;
}

void CRenderWorld::addDrawGeometry(const utl::vector<FGeometryBatch>& new_geometry)
{
	m_vRawGeometryBatches.insert(m_vRawGeometryBatches.end(), new_geometry.begin(), new_geometry.end());

	// Sort drawings by vbo, material and node
	std::sort(m_vRawGeometryBatches.begin(), m_vRawGeometryBatches.end(), [](const FGeometryBatch& a, const FGeometryBatch& b) { return a.vboID < b.vboID; });
}

void CRenderWorld::begin(f32 dt)
{
	// Clear old frame data
	m_vGeometryBatches.clear();

	prepareIndirectCommands();
	//prepareMeshletCommands();

	// Copy frustum sides to gpu buffer
	{
		auto& frustum_buffer = getDrawBuffer(ERenderBufferIndices::eFrustumBuffer);
		for (const auto& frustum : m_vSceneFrustums)
			frustum_buffer->copy(frustum.frustumSides);
	}

	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& texture_remap_data = render_context->getGlobalTextureRemap();

	auto& texture_remap_buffer = getDrawBuffer(ERenderBufferIndices::eTextureRemapBuffer);
	texture_remap_buffer->copy(texture_remap_data);

	for (auto& buffer : m_vBuffers)
		buffer->upload();
}

void CRenderWorld::end()
{
	m_stats.directionalLightCount = 0ull;
	m_stats.pointLightCount = 0ull;
	m_stats.spotLightCount = 0ull;
	m_stats.lightCount = 0ull;

	m_stats.cascadedShadowCount = 0ull;
	m_stats.omniShadowCount = 0ull;
	m_stats.spotShadowCount = 0ull;

	m_stats.meshCount = 0ull;
	m_stats.matricesCount = 0ull;

	m_stats.drawCommandCount = 0ull;

	m_vRawGeometryBatches.clear();
}

u64 CRenderWorld::updateLight(const FLightCommit& light)
{
	auto& lights_buffer = getDrawBuffer(ERenderBufferIndices::eLightBuffer);

	if (light.type == light_type_directional)
		++m_stats.directionalLightCount;
	else if (light.type == light_type_point)
		++m_stats.pointLightCount;
	else if (light.type == light_type_spot)
		++m_stats.spotLightCount;

	lights_buffer->emplace_back(light);

	return m_stats.lightCount++;
}

void CRenderWorld::addShadow(const FCascadedShadowCommit& commit)
{
	auto& shadows_buffer = getDrawBuffer(ERenderBufferIndices::eCascadedShadowBuffer);

	if (shadows_buffer->capacity() < m_stats.cascadedShadowCount + 1ull)
		shadows_buffer->resize((m_stats.cascadedShadowCount + 1ull) * 2ull);

	shadows_buffer->set(m_stats.cascadedShadowCount++, commit);
}

void CRenderWorld::addShadow(const FOmniShadowCommit& commit)
{
	auto& shadows_buffer = getDrawBuffer(ERenderBufferIndices::eOmniShadowBuffer);

	if (shadows_buffer->capacity() < m_stats.omniShadowCount + 1ull)
		shadows_buffer->resize((m_stats.omniShadowCount + 1ull) * 2ull);

	shadows_buffer->set(m_stats.omniShadowCount++, commit);
}

void CRenderWorld::addShadow(const FSpotShadowCommit& commit)
{
	auto& shadows_buffer = getDrawBuffer(ERenderBufferIndices::eSpotShadowBuffer);

	if (shadows_buffer->capacity() < m_stats.spotShadowCount + 1ull)
		shadows_buffer->resize((m_stats.spotShadowCount + 1ull) * 2ull);

	shadows_buffer->set(m_stats.spotShadowCount++, commit);
}

void CRenderWorld::addMatrices(const utl::vector<glm::mat4>& matrices)
{
	auto& matrices_buffer = getDrawBuffer(ERenderBufferIndices::eMatricesBuffer);
	matrices_buffer->copy(matrices);
	m_stats.matricesCount += matrices.size();
}

const glm::mat4& CRenderWorld::getMatrix(u64 index) const
{
	auto& matrices_buffer = getDrawBuffer(ERenderBufferIndices::eMatricesBuffer);
	return *(matrices_buffer->at<glm::mat4>(index));
}

u64 CRenderWorld::updateNode(u32 node_id, const FSceneNode& node)
{
	auto& nodes_buffer = getDrawBuffer(ERenderBufferIndices::eNodeBuffer);

	if (m_nodeRemap.find(node_id) == m_nodeRemap.end())
		m_nodeRemap[node_id] = nodes_buffer->pos();

	nodes_buffer->emplace_back(node);

	return m_nodeRemap[node_id];
}

void CRenderWorld::addMaterial(const FMaterial& material)
{
	auto& materials_buffer = getDrawBuffer(ERenderBufferIndices::eMaterialBuffer);
	materials_buffer->emplace_back(material);
}

void CRenderWorld::addMaterials(const utl::vector<FMaterial>& materials)
{
	auto& materials_buffer = getDrawBuffer(ERenderBufferIndices::eMaterialBuffer);
	materials_buffer->copy(materials);
}

const FMaterial& CRenderWorld::getMaterial(u64 index) const
{
	auto& materials_buffer = getDrawBuffer(ERenderBufferIndices::eMaterialBuffer);
	return *materials_buffer->at<FMaterial>(index);
}

void CRenderWorld::addMesh(const FMesh& mesh)
{
	auto& mesh_buffer = getDrawBuffer(ERenderBufferIndices::eMeshBuffer);
	mesh_buffer->emplace_back(mesh);
	m_stats.meshCount++;
}

void CRenderWorld::addMeshes(const utl::vector<FMesh>& meshes)
{
	auto& mesh_buffer = getDrawBuffer(ERenderBufferIndices::eMeshBuffer);
	mesh_buffer->copy(meshes);
	m_stats.meshCount += meshes.size();
}

const std::unique_ptr<IGPUVector>& CRenderWorld::getDrawBuffer(ERenderBufferIndices buffer_index) const
{
	return m_vBuffers.at(utl::enum_cast(buffer_index));
}

const utl::vector<FGeometryCPUBatch>& CRenderWorld::getGeometryBatches() const
{
	return m_vGeometryBatches;
}

const utl::vector<FDrawInfo>& CRenderWorld::getDrawCommandByType(ERenderGeometryCullerType type) const
{
	static utl::vector<FDrawInfo> empty{};

	auto found = m_perCullerDrawCommands.find(type);
	if (found != m_perCullerDrawCommands.end())
		return found->second;

	return empty;
}

FPostProcess& CRenderWorld::getPostProcessingData()
{
	return m_postprocess;
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
	auto& indirect_count_buffer = getDrawBuffer(ERenderBufferIndices::eCountBuffer);
	auto& raw_gpu_batches_buffer = getDrawBuffer(ERenderBufferIndices::eRawDrawMeshInfoBuffer);

	m_perCullerDrawCommands.clear();
	m_vGeometryBatches.clear();
	
	utl::vector<FGeometryBatch> gpu_batches{};

	u64 offset{ 0ull };
	for (const auto& culler : m_vCullingInfo)
	{
		auto& draw_infos = m_perCullerDrawCommands[culler.type];
		auto& draw_info = draw_infos.emplace_back();
		draw_info.first = m_vGeometryBatches.size();
		draw_info.drawCount = m_vRawGeometryBatches.size();
		draw_info.cullerID = culler.cullerID;

		utl::vector<FGeometryCPUBatch> batches{};
		for (const auto& batch : m_vRawGeometryBatches)
		{
			// Create batch or get already existing
			FGeometryCPUBatch* current_batch{ nullptr };
			if (!batches.empty() && batches.back().vboID == batch.vboID)
				current_batch = &batches.back();
			else
			{
				auto& new_batch = batches.emplace_back();
				new_batch.vboID = batch.vboID;
				new_batch.command.bufferId = indirect_data_buffer->buffer()->handle();
				new_batch.command.offset = offset * m_vRawGeometryBatches.size() * sizeof(FDrawIndexedIndirectData);
				new_batch.command.countBufferId = indirect_count_buffer->buffer()->handle();
				new_batch.command.countOffset = offset * sizeof(u32);
				new_batch.command.maxDrawCount = m_vRawGeometryBatches.size();
				new_batch.command.stride = sizeof(FDrawIndexedIndirectData);

				current_batch = &new_batch;
			}

			//// Check for range is already exists in command list
			//u64 range_hash = utl::murmur_hash_64(&batch.range, sizeof(FGeometryDrawRange), 0ull);
			//if (auto found = indirect_command_cache.find(range_hash); found != indirect_command_cache.end())
			//{
			//	auto& indirect_command = indirect_data.at(found->second);
			//	indirect_command.instanceCount++;
			//}
			//else
			//{
			//	// Cache current command
			//	indirect_command_cache.emplace(range_hash, indirect_data.size());
			//
			//	auto& vbo = m_pRenderInterface->getVertexHandler(batch.vboID);
			//
			//	auto last_index = vbo->getLastIndex();
			//	auto last_vertex = vbo->getLastVertex();
			//
			//	// Create new draw command
			//	auto& indirect_command = indirect_data.emplace_back();
			//	indirect_command.indexCount = batch.range.indexCount == 0ull ? last_index : batch.range.indexCount;;
			//	indirect_command.instanceCount = 1u;
			//	indirect_command.firstIndex = batch.range.beginIndex;
			//	indirect_command.vertexOffset = batch.range.beginVertex;
			//	indirect_command.firstInstance = 0u;
			//	batch.range.vertexCount == 0ull ? last_vertex : batch.range.vertexCount;
			//
			//	current_batch->command.count++;
			//}

			auto& gpu_batch = gpu_batches.emplace_back();
			gpu_batch.vboID = batch.vboID;
			gpu_batch.meshID = batch.meshID;
			gpu_batch.nodeID = batch.nodeID;
			gpu_batch.cameraID = culler.cullerID;
		}

		draw_info.count = batches.size();
		m_vGeometryBatches.insert(m_vGeometryBatches.end(), batches.begin(), batches.end());

		offset++;
	}

	// Count max possible draw command count
	m_stats.cullerCount = m_vCullingInfo.size();
	m_stats.maxDrawCommands = m_vRawGeometryBatches.size();
	m_stats.drawCommandCount = m_stats.maxDrawCommands * m_vCullingInfo.size();
	//m_stats.drawCommandCount = m_stats.maxDrawCommands * (1 + max_directional_light_shadow_count * cascade_shadow_map_cascade_count + max_spot_light_shadow_count + max_point_light_shadow_count * 6);
	if (indirect_data_buffer->capacity() < m_stats.drawCommandCount)
		indirect_data_buffer->resize(m_stats.drawCommandCount * 1.1f);

	raw_gpu_batches_buffer->copy(gpu_batches);

	// Resize output buffer
	auto& gpu_batches_buffer = getDrawBuffer(ERenderBufferIndices::eDrawMeshInfoBuffer);
	if (raw_gpu_batches_buffer->capacity() > gpu_batches_buffer->capacity())
		gpu_batches_buffer->resize(raw_gpu_batches_buffer->capacity());

	if (indirect_count_buffer->capacity() < m_stats.cullerCount)
		indirect_count_buffer->resize(m_stats.cullerCount * 1.1f);
}

void CRenderWorld::prepareMeshletCommands()
{
	//auto& indirect_data_buffer = getDrawBuffer(ERenderBufferIndices::eMeshCommandBuffer);
	//auto& cluster_info_buffer = getDrawBuffer(ERenderBufferIndices::eClusterInfoBuffer);
	//auto& gpu_batches_buffer = getDrawBuffer(ERenderBufferIndices::eRawDrawMeshInfoBuffer);
	//
	//utl::vector<FDrawMeshIndirectData> indirect_data{};
	//utl::vector<FDrawMeshinfo> gpu_batches{};
	//utl::vector<FClusterInfo> gpu_clusters{};
	//
	//utl::hash_map<u64, u64> indirect_command_cache{};
	//
	//for (const auto& batch : m_vRawGeometryBatches)
	//{
	//	FGeometryCPUBatch* current_batch{ nullptr };
	//	if (!m_vGeometryBatches.empty() && m_vGeometryBatches.back().vboID == batch.drawInfo.vboID)
	//		current_batch = &m_vGeometryBatches.back();
	//	else
	//	{
	//		auto& new_batch = m_vGeometryBatches.emplace_back();
	//		new_batch.vboID = batch.drawInfo.vboID;
	//		new_batch.command.bufferId = indirect_data_buffer->buffer()->handle();
	//		new_batch.command.offset = indirect_data.size() * sizeof(FDrawMeshIndirectData);
	//		new_batch.command.count = 0;
	//		new_batch.command.stride = sizeof(FDrawMeshIndirectData);
	//
	//		current_batch = &new_batch;
	//	}
	//
	//	// Create draw call data
	//	for (u64 cidx = 0ull; cidx < batch.range.meshletCount; ++cidx)
	//	{
	//		auto& cluster_info = gpu_clusters.emplace_back();
	//		cluster_info.drawID = gpu_batches.size();
	//		cluster_info.firstMeshlet = batch.range.firstMeshlet;
	//		cluster_info.meshletCount = batch.range.meshletCount;
	//	}
	//
	//	//u64 range_hash = utl::utils::murmur_hash_64(&batch.range, sizeof(FGeometryDrawRange), 0ull);
	//	//if (auto found = indirect_command_cache.find(range_hash); found != indirect_command_cache.end())
	//	//{
	//	//	auto& indirect_command = indirect_data.at(found->second);
	//	//	indirect_command.instanceCount++;
	//	//}
	//	//else
	//	//{
	//		// Cache current command
	//	//	indirect_command_cache.emplace(range_hash, indirect_data.size());
	//
	//		auto& vbo = m_pRenderInterface->getVertexHandler(batch.drawInfo.vboID);
	//
	//		auto last_index = vbo->getLastIndex();
	//		auto last_vertex = vbo->getLastVertex();
	//
	//		// Create new draw command
	//		auto& indirect_command = indirect_data.emplace_back();
	//		indirect_command.firstTask = 0;
	//		indirect_command.taskCount = (batch.range.meshletCount + kShaderGroupSize - 1) / kShaderGroupSize;
	//
	//		current_batch->command.count++;
	//	//}
	//
	//	// Save draw order in all draw array
	//	gpu_batches.emplace_back(batch.drawInfo);
	//}
	//
	//m_stats.drawCommandCount = indirect_data.size();
	//
	//indirect_data_buffer->copy(indirect_data);
	//cluster_info_buffer->copy(gpu_clusters);
	//gpu_batches_buffer->copy(gpu_batches);
}