#include "RenderWorld.h"

#include "BufferObject.h"

using namespace engine::render;

constexpr const u64 MAX_BATCH_RANGE_SIZE{ 1024ull };
constexpr const u64 MAX_RENDER_OBJECT_COUNT{ 32768ull };
constexpr const u64 MAX_PRIMITIVE_COUNT{ 32768ull };
constexpr const u64 MAX_MATERIAL_COUNT{ 32768ull };
constexpr const u64 MAX_INDIRECT_DRAW_COUNT = 
					MAX_PRIMITIVE_COUNT + 
					MAX_PRIMITIVE_COUNT * max_directional_light_shadow_count * cascade_shadow_map_cascade_count +
					MAX_PRIMITIVE_COUNT * max_spot_light_shadow_count +
					MAX_PRIMITIVE_COUNT * max_point_light_shadow_count;

template<class _Ty>
stl::unique_ptr<IBufferObject> createDrawBuffer(const stl::string& name, EBufferTypeFlagBits type, u64 count, IRenderInterface* render_interface)
{
	FBufferCreateInfo buffer_ci{};
	buffer_ci.flags = type;
	buffer_ci.size = sizeof(_Ty);
	buffer_ci.count = count;

	auto new_buffer = stl::make_unique<CBufferObject>(render_interface);
	new_buffer->create(name, buffer_ci);
	return new_buffer;
}

CRenderWorld::CRenderWorld(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CRenderWorld::create()
{
	m_vBuffers.resize(utl::enum_cast(ERenderBufferIndices::eLastIndex));

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eNodeBuffer)) = createDrawBuffer<FSceneNode>("NodeStorage", EBufferTypeFlagBits::eStorage, MAX_PRIMITIVE_COUNT, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eMaterialBuffer)) = createDrawBuffer<FMaterial>("MaterialStorage", EBufferTypeFlagBits::eStorage, MAX_MATERIAL_COUNT, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eSkinBuffer)) = createDrawBuffer<glm::mat4>("SkinningJoints", EBufferTypeFlagBits::eStorage, MAX_PRIMITIVE_COUNT * 128ull, m_pRenderInterface);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eDirectionalLightBuffer)) = createDrawBuffer<FDirectionalLightCommit>("DirectionalLights", EBufferTypeFlagBits::eStorage, max_directional_light_count, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::ePointLightBuffer)) = createDrawBuffer<FPointLightCommit>("PointLights", EBufferTypeFlagBits::eStorage, max_point_light_count, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eSpotLightBuffer)) = createDrawBuffer<FSpotLightCommit>("SpotLights", EBufferTypeFlagBits::eStorage, max_spot_light_count, m_pRenderInterface);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eCascadedShadowBuffer)) = createDrawBuffer<FCascadedShadowCommit>("CascadedShadows", EBufferTypeFlagBits::eStorage, max_directional_light_shadow_count, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eOmniShadowBuffer)) = createDrawBuffer<FOmniShadowCommit>("OmniShadows", EBufferTypeFlagBits::eStorage, max_point_light_shadow_count, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eSpotShadowBuffer)) = createDrawBuffer<FSpotShadowCommit>("SpotShadows", EBufferTypeFlagBits::eStorage, max_spot_light_shadow_count, m_pRenderInterface);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eDrawBatchBuffer)) = createDrawBuffer<FGeometryGPUBatch>("GeometryBatches", EBufferTypeFlagBits::eStorage, MAX_RENDER_OBJECT_COUNT, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eCullingInfoBuffer)) = createDrawBuffer<FGeometryGPUCullInfo>("GeometryCullingInfo", EBufferTypeFlagBits::eStorage, MAX_RENDER_OBJECT_COUNT, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eFrustumBuffer)) = createDrawBuffer<glm::vec4>("FrustumSides", EBufferTypeFlagBits::eStorage, MAX_PRIMITIVE_COUNT * 6ull, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eDrawCommandBuffer)) = createDrawBuffer<FDrawIndexedIndirectData>("IndexedDrawIndirectBuffer", EBufferTypeFlagBits::eIndirect, MAX_INDIRECT_DRAW_COUNT, m_pRenderInterface);

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
	stl::sort(m_vRawGeometryBatches.begin(), m_vRawGeometryBatches.end(), [](const FGeometryRawBatch& a, const FGeometryRawBatch& b)
		{
			if (a.callInfo.vboID == b.callInfo.vboID)
			{
				if (a.callInfo.materialID == b.callInfo.materialID)
					return a.callInfo.instanceID < b.callInfo.instanceID;

				return a.callInfo.materialID < b.callInfo.materialID;
			}

			return a.callInfo.vboID < b.callInfo.vboID;
		});
}

void CRenderWorld::render(f32 dt)
{
	// Clear old frame data
	m_vGeometryBatches.clear();

	for (auto& [type, range] : m_mBatchRanges)
		range.clear();

	stl::vector<FGeometryGPUCullInfo> gpu_culling_infos{};
	stl::vector<FDrawIndexedIndirectData> indirect_data{};
	stl::vector<FGeometryGPUBatch> gpu_batches{};
	stl::vector<u32> counted{};

	u64 command_offset{ 0ull };
	u64 batch_offset{ 0ull };

	// Iterate whole cullers
	for (const auto& cull_info : m_vCullingInfo)
	{
		// Generate raw batches by material chain
		u64 current_material{ ~0ull };
		u64 current_node{ ~0ull };
		for (const auto& batch : m_vRawGeometryBatches)
		{
			auto& gpu_culling_info = gpu_culling_infos.emplace_back();
			gpu_culling_info.frustumID = cull_info.cullerID;
			gpu_culling_info.instanceID = batch.callInfo.instanceID;

			auto& vbo = m_pRenderInterface->getVertexHandler(batch.callInfo.vboID);

			auto last_index = vbo->getLastIndex();
			auto last_vertex = vbo->getLastVertex();

			auto& indirect_command = indirect_data.emplace_back();
			indirect_command.indexCount = batch.range.indexCount == 0ull ? last_index : batch.range.indexCount;;
			indirect_command.instanceCount = 1u;
			indirect_command.firstIndex = batch.range.beginIndex;
			indirect_command.vertexOffset = batch.range.beginVertex;
			indirect_command.firstInstance = 0u;
			batch.range.vertexCount == 0ull ? last_vertex : batch.range.vertexCount;

			indirect_command.aabb.min = batch.range.bounds.min;
			indirect_command.aabb.max = batch.range.bounds.max;

			// Make compact batches by material chain
			if (!gpu_batches.empty() &&
				gpu_batches.back().vboID == batch.callInfo.vboID &&
				//gpu_batches.back().instanceID == batch.callInfo.instanceID &&
				//(current_material + 1 == batch.callInfo.materialID || batch.callInfo.materialID == current_material) &&
				batch.callInfo.materialID == current_material &&
				current_node + 1 == batch.callInfo.instanceID)
			{
				counted.back()++;
			}
			else
			{
				auto& new_gpu_batch = gpu_batches.emplace_back();
				new_gpu_batch = batch.callInfo;
				counted.emplace_back(1ull);
			}

			current_material = batch.callInfo.materialID;
			current_node = batch.callInfo.instanceID;
		}

		// Create new batch ranges for current culler type
		auto& batch_ranges = m_mBatchRanges[cull_info.type];
		auto& batch_range = batch_ranges.emplace_back();
		batch_range.offset = m_vGeometryBatches.size();
		
		// Prepare cpu data
		for (u64 idx = batch_offset; idx < gpu_batches.size(); ++idx)
		{
			auto& gpu_batch = gpu_batches.at(idx);
		
			auto& new_cpu_batch = m_vGeometryBatches.emplace_back();
			new_cpu_batch.vboID = gpu_batch.vboID;
			new_cpu_batch.command.bufferId = getDrawBuffer(ERenderBufferIndices::eDrawCommandBuffer)->handle();
			new_cpu_batch.command.offset = command_offset * sizeof(FDrawIndexedIndirectData);
			new_cpu_batch.command.count = counted.at(idx);
			new_cpu_batch.command.stride = sizeof(FDrawIndexedIndirectData);
		
			command_offset += new_cpu_batch.command.count;
		}
		
		batch_offset += gpu_batches.size() - batch_offset;
		batch_range.count = m_vGeometryBatches.size() - batch_range.offset;
	}

	m_vRawGeometryBatches.clear();
	m_stats.drawCommandCount = command_offset;

	auto& batch_buffer = getDrawBuffer(ERenderBufferIndices::eDrawBatchBuffer);
	batch_buffer->map();
	batch_buffer->write(gpu_batches.data(), gpu_batches.size());
	batch_buffer->unmap();

	auto& culling_buffer = getDrawBuffer(ERenderBufferIndices::eCullingInfoBuffer);
	culling_buffer->map();
	culling_buffer->write(gpu_culling_infos.data(), gpu_culling_infos.size());
	culling_buffer->unmap();

	// Copy frustum sides to gpu buffer
	{
		auto& frustum_buffer = getDrawBuffer(ERenderBufferIndices::eFrustumBuffer);
		frustum_buffer->map();

		u64 offset{ 0ull };
		for (const auto& frustum : m_vSceneFrustums)
		{
			for (const auto& side : frustum.frustumSides)
			{
				frustum_buffer->write(&side, sizeof(glm::vec4), offset);
				offset += sizeof(glm::vec4);
			}
		}

		frustum_buffer->unmap();
	}

	auto& material_buffer = getDrawBuffer(ERenderBufferIndices::eMaterialBuffer);
	material_buffer->map();
	material_buffer->write(m_vMaterials.data(), m_vMaterials.size());
	material_buffer->unmap();

	auto& node_buffer = getDrawBuffer(ERenderBufferIndices::eNodeBuffer);
	node_buffer->map();
	node_buffer->write(m_vNodes.data(), m_vNodes.size());
	node_buffer->unmap();

	auto& skin_buffer = getDrawBuffer(ERenderBufferIndices::eSkinBuffer);
	skin_buffer->map();
	skin_buffer->write(m_vSkins.data(), m_vSkins.size());
	skin_buffer->unmap();

	auto& directional_light_buffer = getDrawBuffer(ERenderBufferIndices::eDirectionalLightBuffer);
	directional_light_buffer->map();
	directional_light_buffer->write(m_vDirectionalLights.data(), m_vDirectionalLights.size());
	directional_light_buffer->unmap();

	auto& point_light_buffer = getDrawBuffer(ERenderBufferIndices::ePointLightBuffer);
	point_light_buffer->map();
	point_light_buffer->write(m_vPointLights.data(), m_vPointLights.size());
	point_light_buffer->unmap();

	auto& spot_light_buffer = getDrawBuffer(ERenderBufferIndices::eSpotLightBuffer);
	spot_light_buffer->map();
	spot_light_buffer->write(m_vSpotLights.data(), m_vSpotLights.size());
	spot_light_buffer->unmap();

	auto& cascaded_shadow_buffer = getDrawBuffer(ERenderBufferIndices::eCascadedShadowBuffer);
	cascaded_shadow_buffer->map();
	cascaded_shadow_buffer->write(m_vCascadedShadows.data(), m_vCascadedShadows.size());
	cascaded_shadow_buffer->unmap();

	auto& omni_shadow_buffer = getDrawBuffer(ERenderBufferIndices::eOmniShadowBuffer);
	omni_shadow_buffer->map();
	omni_shadow_buffer->write(m_vOmniShadows.data(), m_vOmniShadows.size());
	omni_shadow_buffer->unmap();

	auto& spot_shadow_buffer = getDrawBuffer(ERenderBufferIndices::eSpotShadowBuffer);
	spot_shadow_buffer->map();
	spot_shadow_buffer->write(m_vSpotShadows.data(), m_vSpotShadows.size());
	spot_shadow_buffer->unmap();

	auto& draw_command_buffer = getDrawBuffer(ERenderBufferIndices::eDrawCommandBuffer);
	draw_command_buffer->map();
	draw_command_buffer->write(indirect_data.data(), indirect_data.size());
	draw_command_buffer->unmap();

	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& texture_remap_data = render_context->getGlobalTextureRemap();
	auto& texture_remap_buffer = getDrawBuffer(ERenderBufferIndices::eTextureRemapBuffer);
	texture_remap_buffer->map();
	texture_remap_buffer->write(texture_remap_data.data(), texture_remap_data.size());
	texture_remap_buffer->unmap();
}

void CRenderWorld::addLight(const FDirectionalLightCommit& light)
{
	if (m_vDirectionalLights.size() < m_stats.directionalLightCount + 1ull)
		m_vDirectionalLights.resize((m_stats.directionalLightCount + 1ull) * 2ull);

	m_vDirectionalLights.at(m_stats.directionalLightCount++) = light;
}

void CRenderWorld::addLight(const FPointLightCommit& light)
{
	if (m_vPointLights.size() < m_stats.pointLightCount + 1ull)
		m_vPointLights.resize((m_stats.pointLightCount + 1ull) * 2ull);

	m_vPointLights.at(m_stats.pointLightCount++) = light;
}

void CRenderWorld::addLight(const FSpotLightCommit& light)
{
	if (m_vSpotLights.size() < m_stats.spotLightCount + 1ull)
		m_vSpotLights.resize((m_stats.spotLightCount + 1ull) * 2ull);

	m_vSpotLights.at(m_stats.spotLightCount++) = light;
}

void CRenderWorld::addShadow(const FCascadedShadowCommit& commit)
{
	if (m_vCascadedShadows.size() < m_stats.cascadedShadowCount + 1ull)
		m_vCascadedShadows.resize((m_stats.cascadedShadowCount + 1ull) * 2ull);

	m_vCascadedShadows.at(m_stats.cascadedShadowCount++) = commit;
}

void CRenderWorld::addShadow(const FOmniShadowCommit& commit)
{
	if (m_vOmniShadows.size() < m_stats.omniShadowCount + 1ull)
		m_vOmniShadows.resize((m_stats.omniShadowCount + 1ull) * 2ull);

	m_vOmniShadows.at(m_stats.omniShadowCount++) = commit;
}

void CRenderWorld::addShadow(const FSpotShadowCommit& commit)
{
	if (m_vSpotShadows.size() < m_stats.spotShadowCount + 1ull)
		m_vSpotShadows.resize((m_stats.spotShadowCount + 1ull) * 2ull);

	m_vSpotShadows.at(m_stats.spotShadowCount++) = commit;
}

void CRenderWorld::updateNode(u32 node_id, const FSceneNode& node)
{
	if (m_vNodes.size() < node_id + 1ull)
		m_vNodes.resize((node_id + 1ull) * 2ull);

	m_vNodes.at(node_id) = node;
}

void CRenderWorld::updateSkin(const stl::array<glm::mat4, 128ull>& matrices)
{
	if (m_vSkins.size() < (m_stats.skinCount * 128ull) + 1ull)
		m_vSkins.resize(((m_stats.skinCount * 128ull) + 1ull) * 2ull);

	auto skin_begin = m_vSkins.begin() + (m_stats.skinCount * 128ull);
	for (u64 idx = 0ull; idx < 128ull; ++idx)
		m_vSkins.at(m_stats.skinCount * 128ull + idx) = matrices.at(idx);

	m_stats.skinCount++;
}

void CRenderWorld::addMaterial(const FMaterial& material)
{
	m_vMaterials.emplace_back(material);
}

void CRenderWorld::addMaterials(const stl::vector<FMaterial>& materials)
{
	m_vMaterials.insert(m_vMaterials.end(), materials.begin(), materials.end());
}

const stl::unique_ptr<IBufferObject>& CRenderWorld::getDrawBuffer(ERenderBufferIndices buffer_index)
{
	return m_vBuffers.at(utl::enum_cast(buffer_index));
}

//const stl::vector<stl::unique_ptr<IBufferObject>>& CRenderWorld::getDrawBuffers()
//{
//	return m_vBuffers;
//}

const stl::unordered_map<ERenderGeometryCullerType, stl::vector<FGeometryBatchRange>>& CRenderWorld::getAllGeometryRanges() const
{
	return m_mBatchRanges;
}

const stl::vector<FGeometryBatchRange>& CRenderWorld::getGeometryRanges(ERenderGeometryCullerType type) const
{
	return m_mBatchRanges[type];
}

const FGeometryCPUBatch& CRenderWorld::getGeometryBatch(u64 index) const
{
	return m_vGeometryBatches.at(index);
}

FRenderWorldStats& CRenderWorld::getStats()
{
	return m_stats;
}

const FRenderWorldStats& CRenderWorld::getStats() const
{
	return m_stats;
}