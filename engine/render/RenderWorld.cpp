#include "RenderWorld.h"

#include "GPUVector.h"
#include "RangeGPUVector.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

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

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eMatricesBuffer)) = std::make_unique<CRangeGPUVector>(render_interface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eCameraBuffer)) = std::make_unique<CRangeGPUVector>(render_interface);
}

void CRenderWorld::Create()
{
	m_vBuffers.resize(utl::enum_cast(ERenderBufferIndices::eLastIndex));

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eNodeBuffer))->Create<FSceneNode>("NodeStorage", 16ull, EBufferUsageFlagBits::eStorageBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eMatricesBuffer))->Create<glm::mat4>("MatricesBuffer", 16ull, EBufferUsageFlagBits::eStorageBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eMeshBuffer))->Create<FMesh>("MeshBuffer", 16ull, EBufferUsageFlagBits::eStorageBuffer);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eMaterialBuffer))->Create<FMaterial>("MaterialStorage", 16ull, EBufferUsageFlagBits::eStorageBuffer);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eLightBuffer))->Create<FLightCommit>("LightsBuffer", max_directional_light_count, EBufferUsageFlagBits::eStorageBuffer);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eCascadedShadowBuffer))->Create<FCascadedShadowCommit>("CascadedShadows", max_directional_light_shadow_count, EBufferUsageFlagBits::eStorageBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eOmniShadowBuffer))->Create<FOmniShadowCommit>("OmniShadows", max_point_light_shadow_count, EBufferUsageFlagBits::eStorageBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eSpotShadowBuffer))->Create<FSpotShadowCommit>("SpotShadows", max_spot_light_shadow_count, EBufferUsageFlagBits::eStorageBuffer);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eDrawMeshInfoBuffer))->Create<FGeometryBatch>("GeometryBatches", 16ull, EBufferUsageFlagBits::eStorageBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eRawDrawMeshInfoBuffer))->Create<FGeometryBatch>("RawGeometryBatches", 16ull, EBufferUsageFlagBits::eStorageBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eClusterInfoBuffer))->Create<FClusterInfo>("ClusterInfoBuffer", 16ull, EBufferUsageFlagBits::eStorageBuffer);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eFrustumBuffer))->Create<glm::vec4>("FrustumSides", 16ull, EBufferUsageFlagBits::eStorageBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eDrawCommandBuffer))->Create<FDrawIndexedIndirectData>("IndexedDrawIndirectBuffer", 16ull, EBufferUsageFlagBits::eIndirectBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eCountBuffer))->Create<FDrawIndexedIndirectData>("IndirectCountBuffer", 16ull, EBufferUsageFlagBits::eIndirectBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eMeshCommandBuffer))->Create<FDrawMeshIndirectData>("MeshIndirectBuffer", 16ull, EBufferUsageFlagBits::eIndirectBuffer);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eTextureRemapBuffer))->Create<uint32_t>("TextureBindingRemap", 16ull, EBufferUsageFlagBits::eStorageBuffer);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eCameraBuffer))->Create<FCamera>("CameraBuffer", 16ull, EBufferUsageFlagBits::eStorageBuffer);
}

u64 CRenderWorld::RegisterCamera(ERenderGeometryCullerType type)
{
	auto& camera_buffer = GetDrawBuffer(ERenderBufferIndices::eCameraBuffer);
	auto& frustum_buffer = GetDrawBuffer(ERenderBufferIndices::eFrustumBuffer);
	auto& matrices_buffer = GetDrawBuffer(ERenderBufferIndices::eMatricesBuffer);

	auto& new_camera = m_vCameras.emplace_back();
	new_camera.cameraId = camera_buffer->AddByteBlock();
	new_camera.matricesId = matrices_buffer->AddByteBlock(matrix_offset_count);
	new_camera.frustumId = frustum_buffer->AddByteBlock(4u);
	new_camera.type = type;

	//camera.type = type == ERenderGeometryCullerType::eCascadedShadow; // I think that it's "ortho"/"perspective"

	return m_vCameras.size() - 1ull;
}

void CRenderWorld::UpdateCamera(u64 camera_id, const glm::vec3& position, const glm::quat& rotation, const glm::vec2& size, f32 fov, f32 znear, f32 zfar)
{
	auto& camera_buffer = GetDrawBuffer(ERenderBufferIndices::eCameraBuffer);
	auto& frustum_buffer = GetDrawBuffer(ERenderBufferIndices::eFrustumBuffer);
	auto& matrices_buffer = GetDrawBuffer(ERenderBufferIndices::eMatricesBuffer);

	auto& camera = m_vCameras[camera_id];

	const std::array<std::pair<const void*, u64>, 2u> view_params{
		std::make_pair(&position[0], sizeof(position)),
		std::make_pair(&rotation[0], sizeof(rotation))
	};

	const std::array<std::pair<const void*, u64>, 4u> proj_params{
		std::make_pair(&size[0], sizeof(size)),
		std::make_pair(&fov, sizeof(fov)),
		std::make_pair(&znear, sizeof(znear)),
		std::make_pair(&zfar, sizeof(zfar))
	};

	bool need_update_view{ false };
	for (u64 idx = 0ull; idx < view_params.size(); ++idx)
	{
		const auto& param = view_params[idx];
		auto hash = utl::murmur_hash((const char*)param.first, param.second, 0u);
		if (camera.paramHash[idx] != hash)
		{
			need_update_view = true;
			camera.paramHash[idx] = hash;
		}
	}

	bool need_update_projection{ false };
	for (u64 idx = 0ull; idx < proj_params.size(); ++idx)
	{
		const auto& param = proj_params[idx];
		auto hash = utl::murmur_hash((const char*)param.first, param.second, 0u);
		if (camera.paramHash[idx + view_params.size()] != hash)
		{
			need_update_projection = true;
			camera.paramHash[idx + view_params.size()] = hash;
		}
	}

	auto matrices = matrices_buffer->GetRange<glm::mat4>(camera.matricesId);

	glm::mat4 prev_view{};
	if (need_update_view)
	{
		glm::vec3 forward = glm::normalize(glm::rotate(rotation, glm::vec3(0.f, 0.f, -1.f)));
		glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3{ 0.f, 1.f, 0.f }));
		glm::vec3 up = glm::normalize(glm::cross(right, forward));

		prev_view = matrices[view_matrix_offset];
		matrices[view_matrix_offset] = glm::lookAt(position, position + forward, up);
		matrices[inv_view_matrix_offset] = glm::inverse(matrices[view_matrix_offset]);
	}

	glm::mat4 prev_projection{};
	if (need_update_projection)
	{
		prev_projection = matrices[projection_matrix_offset];
		matrices[projection_matrix_offset] = glm::perspective(fov, size.x / size.y, znear, zfar); // TODO: ortho
		matrices[inv_projection_matrix_offset] = glm::inverse(matrices[projection_matrix_offset]);
	}

	if (need_update_view || need_update_projection)
	{
		// Update camera frustum
		auto frustum_range = frustum_buffer->GetRange<glm::vec4>(camera.frustumId);
		camera.frustum.update(matrices[view_matrix_offset], matrices[projection_matrix_offset], frustum_range.data());

		matrices[view_projection_matrix_offset] = matrices[projection_matrix_offset] * matrices[view_matrix_offset];
		matrices[inv_view_projection_matrix_offset] = glm::inverse(matrices[view_projection_matrix_offset]);
		matrices[prev_view_projection_matrix_offset] = prev_projection;

		auto& camera_reprojection = matrices[reprojection_matrix_offset];
		camera_reprojection = matrices[prev_view_projection_matrix_offset] * matrices[inv_view_projection_matrix_offset];
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
			-1.0f, 1.0f, linear_z ? -rcp_z_magic : 0.0f, 1.0f
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

		auto& matrices_range = matrices_buffer->GetByteRange(camera.matricesId);

		auto& camera_instance = camera_buffer->Get<FCamera>(camera.cameraId);
		camera_instance.matricesOffset = matrices_range.begin;
		camera_instance.zNear = znear;
		camera_instance.zFar = zfar;
		camera_instance.size = size;
		camera_instance.invSize = 1.f / size;
	}
}

const FRenderCamera& CRenderWorld::GetCamera(u64 camera_id) const
{
	return m_vCameras[camera_id];
}

const FRenderCamera& CRenderWorld::GetDefaultCamera() const
{
	return GetCamera(m_defaultCameraID);
}

const u64 CRenderWorld::GetCameraID(ERenderGeometryCullerType type) const
{
	for (u64 idx = 0ull; idx < m_vCameras.size(); ++idx)
	{
		if (m_vCameras[idx].type == type)
			return idx;
	}

	return m_defaultCameraID;
}

const u64 CRenderWorld::GetDefaultCameraID() const
{
	return m_defaultCameraID;
}

void CRenderWorld::AddDrawGeometry(const utl::vector<FGeometryBatch>& new_geometry)
{
	m_vRawGeometryBatches.insert(m_vRawGeometryBatches.end(), new_geometry.begin(), new_geometry.end());

	// Sort drawings by vbo, material and node
	std::sort(m_vRawGeometryBatches.begin(), m_vRawGeometryBatches.end(), [](const FGeometryBatch& a, const FGeometryBatch& b) { return a.vboID < b.vboID; });
}

void CRenderWorld::Begin(f32 dt)
{
	// Clear old frame data
	m_vGeometryBatches.clear();

	prepareIndirectCommands();
	//prepareMeshletCommands();

	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto& texture_remap_data = render_context->GetGlobalTextureRemap();

	auto& texture_remap_buffer = GetDrawBuffer(ERenderBufferIndices::eTextureRemapBuffer);
	texture_remap_buffer->Copy(texture_remap_data);

	for (auto& buffer : m_vBuffers)
		buffer->Upload();
}

void CRenderWorld::End()
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

u64 CRenderWorld::UpdateLight(const FLightCommit& light)
{
	auto& lights_buffer = GetDrawBuffer(ERenderBufferIndices::eLightBuffer);

	if (light.type == light_type_directional)
		++m_stats.directionalLightCount;
	else if (light.type == light_type_point)
		++m_stats.pointLightCount;
	else if (light.type == light_type_spot)
		++m_stats.spotLightCount;

	lights_buffer->EmplaceBack(light);

	return m_stats.lightCount++;
}

void CRenderWorld::AddShadow(const FCascadedShadowCommit& commit)
{
	auto& shadows_buffer = GetDrawBuffer(ERenderBufferIndices::eCascadedShadowBuffer);

	if (shadows_buffer->Capacity() < m_stats.cascadedShadowCount + 1ull)
		shadows_buffer->Resize((m_stats.cascadedShadowCount + 1ull) * 2ull);

	shadows_buffer->Set(m_stats.cascadedShadowCount++, commit);
}

void CRenderWorld::AddShadow(const FOmniShadowCommit& commit)
{
	auto& shadows_buffer = GetDrawBuffer(ERenderBufferIndices::eOmniShadowBuffer);

	if (shadows_buffer->Capacity() < m_stats.omniShadowCount + 1ull)
		shadows_buffer->Resize((m_stats.omniShadowCount + 1ull) * 2ull);

	shadows_buffer->Set(m_stats.omniShadowCount++, commit);
}

void CRenderWorld::AddShadow(const FSpotShadowCommit& commit)
{
	auto& shadows_buffer = GetDrawBuffer(ERenderBufferIndices::eSpotShadowBuffer);

	if (shadows_buffer->Capacity() < m_stats.spotShadowCount + 1ull)
		shadows_buffer->Resize((m_stats.spotShadowCount + 1ull) * 2ull);

	shadows_buffer->Set(m_stats.spotShadowCount++, commit);
}

void CRenderWorld::AddMatrices(const utl::vector<glm::mat4>& matrices)
{
	auto& matrices_buffer = GetDrawBuffer(ERenderBufferIndices::eMatricesBuffer);
	matrices_buffer->Copy(matrices);
	m_stats.matricesCount += matrices.size();
}

const glm::mat4& CRenderWorld::GetMatrix(u64 index) const
{
	auto& matrices_buffer = GetDrawBuffer(ERenderBufferIndices::eMatricesBuffer);
	return *(matrices_buffer->at<glm::mat4>(index));
}

u64 CRenderWorld::UpdateNode(u32 node_id, const FSceneNode& node)
{
	auto& nodes_buffer = GetDrawBuffer(ERenderBufferIndices::eNodeBuffer);

	if (m_nodeRemap.find(node_id) == m_nodeRemap.end())
		m_nodeRemap[node_id] = nodes_buffer->Pos();

	nodes_buffer->EmplaceBack(node);

	return m_nodeRemap[node_id];
}

void CRenderWorld::AddMaterial(const FMaterial& material)
{
	auto& materials_buffer = GetDrawBuffer(ERenderBufferIndices::eMaterialBuffer);
	materials_buffer->EmplaceBack(material);
}

void CRenderWorld::AddMaterials(const utl::vector<FMaterial>& materials)
{
	auto& materials_buffer = GetDrawBuffer(ERenderBufferIndices::eMaterialBuffer);
	materials_buffer->Copy(materials);
}

const FMaterial& CRenderWorld::GetMaterial(u64 index) const
{
	auto& materials_buffer = GetDrawBuffer(ERenderBufferIndices::eMaterialBuffer);
	return *materials_buffer->at<FMaterial>(index);
}

void CRenderWorld::AddMesh(const FMesh& mesh)
{
	auto& mesh_buffer = GetDrawBuffer(ERenderBufferIndices::eMeshBuffer);
	mesh_buffer->EmplaceBack(mesh);
	m_stats.meshCount++;
}

void CRenderWorld::AddMeshes(const utl::vector<FMesh>& meshes)
{
	auto& mesh_buffer = GetDrawBuffer(ERenderBufferIndices::eMeshBuffer);
	mesh_buffer->Copy(meshes);
	m_stats.meshCount += meshes.size();
}

const std::unique_ptr<IGPUVector>& CRenderWorld::GetDrawBuffer(ERenderBufferIndices buffer_index) const
{
	return m_vBuffers.at(utl::enum_cast(buffer_index));
}

const utl::vector<FGeometryCPUBatch>& CRenderWorld::GetGeometryBatches() const
{
	return m_vGeometryBatches;
}

const utl::vector<FDrawInfo>& CRenderWorld::GetDrawCommandByType(ERenderGeometryCullerType type) const
{
	static utl::vector<FDrawInfo> empty{};

	auto found = m_perCullerDrawCommands.find(type);
	if (found != m_perCullerDrawCommands.end())
		return found->second;

	return empty;
}

FPostProcess& CRenderWorld::GetPostProcessingData()
{
	return m_postprocess;
}

FRenderWorldStats& CRenderWorld::GetStats()
{
	return m_stats;
}

const FRenderWorldStats& CRenderWorld::GetStats() const
{
	return m_stats;
}

void CRenderWorld::prepareIndirectCommands()
{
	auto& indirect_data_buffer = GetDrawBuffer(ERenderBufferIndices::eDrawCommandBuffer);
	auto& indirect_count_buffer = GetDrawBuffer(ERenderBufferIndices::eCountBuffer);
	auto& raw_gpu_batches_buffer = GetDrawBuffer(ERenderBufferIndices::eRawDrawMeshInfoBuffer);

	m_perCullerDrawCommands.clear();
	m_vGeometryBatches.clear();
	
	utl::vector<FGeometryBatch> gpu_batches{};

	u64 offset{ 0ull };
	for (u64 cameraID = 0ull; cameraID < m_vCameras.size(); ++cameraID)
	{
		auto& camera = m_vCameras[cameraID];

		auto& draw_infos = m_perCullerDrawCommands[camera.type];
		auto& draw_info = draw_infos.emplace_back();
		draw_info.first = m_vGeometryBatches.size();
		draw_info.drawCount = m_vRawGeometryBatches.size();
		draw_info.cullerID = cameraID;

		utl::vector<FGeometryCPUBatch> batches{};
		for (const auto& batch : m_vRawGeometryBatches)
		{
			// Create batch or get already existing
			FGeometryCPUBatch* current_batch{ nullptr };
			if (!batches.empty() && batches.back().vboID == FRHIHandle(batch.vboID))
				current_batch = &batches.back();
			else
			{
				auto& new_batch = batches.emplace_back();
				new_batch.vboID = FRHIHandle(batch.vboID);
				new_batch.command.bufferId = indirect_data_buffer->Buffer()->Handle();
				new_batch.command.offset = offset * m_vRawGeometryBatches.size() * sizeof(FDrawIndexedIndirectData);
				new_batch.command.countBufferId = indirect_count_buffer->Buffer()->Handle();
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
			//	auto& vbo = m_pRenderInterface->GetVertexHandler(batch.vboID);
			//
			//	auto last_index = vbo->getLastIndex();
			//	auto last_vertex = vbo->getLastVertex();
			//
			//	// Create new Draw command
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
			gpu_batch.cameraID = cameraID;
		}

		draw_info.count = batches.size();
		m_vGeometryBatches.insert(m_vGeometryBatches.end(), batches.begin(), batches.end());

		offset++;
	}

	// Count max possible Draw command count
	m_stats.cullerCount = m_vCameras.size();
	m_stats.maxDrawCommands = m_vRawGeometryBatches.size();
	m_stats.drawCommandCount = m_stats.maxDrawCommands * m_vCameras.size();
	//m_stats.drawCommandCount = m_stats.maxDrawCommands * (1 + max_directional_light_shadow_count * cascade_shadow_map_cascade_count + max_spot_light_shadow_count + max_point_light_shadow_count * 6);
	if (indirect_data_buffer->Capacity() < m_stats.drawCommandCount)
		indirect_data_buffer->Resize(m_stats.drawCommandCount * 1.1f);

	raw_gpu_batches_buffer->Copy(gpu_batches);

	// Resize output buffer
	auto& gpu_batches_buffer = GetDrawBuffer(ERenderBufferIndices::eDrawMeshInfoBuffer);
	if (raw_gpu_batches_buffer->Capacity() > gpu_batches_buffer->Capacity())
		gpu_batches_buffer->Resize(raw_gpu_batches_buffer->Capacity());

	if (indirect_count_buffer->Capacity() < m_stats.cullerCount)
		indirect_count_buffer->Resize(m_stats.cullerCount * 1.1f);
}

void CRenderWorld::prepareMeshletCommands()
{
	//auto& indirect_data_buffer = GetDrawBuffer(ERenderBufferIndices::eMeshCommandBuffer);
	//auto& cluster_info_buffer = GetDrawBuffer(ERenderBufferIndices::eClusterInfoBuffer);
	//auto& gpu_batches_buffer = GetDrawBuffer(ERenderBufferIndices::eRawDrawMeshInfoBuffer);
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
	//	// Create Draw call data
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
	//		auto& vbo = m_pRenderInterface->GetVertexHandler(batch.drawInfo.vboID);
	//
	//		auto last_index = vbo->getLastIndex();
	//		auto last_vertex = vbo->getLastVertex();
	//
	//		// Create new Draw command
	//		auto& indirect_command = indirect_data.emplace_back();
	//		indirect_command.firstTask = 0;
	//		indirect_command.taskCount = (batch.range.meshletCount + kShaderGroupSize - 1) / kShaderGroupSize;
	//
	//		current_batch->command.count++;
	//	//}
	//
	//	// Save Draw order in all Draw array
	//	gpu_batches.emplace_back(batch.drawInfo);
	//}
	//
	//m_stats.drawCommandCount = indirect_data.size();
	//
	//indirect_data_buffer->copy(indirect_data);
	//cluster_info_buffer->copy(gpu_clusters);
	//gpu_batches_buffer->copy(gpu_batches);
}