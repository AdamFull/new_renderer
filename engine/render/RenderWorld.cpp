#include "RenderWorld.h"

#include "BufferObject.h"

using namespace engine::render;

constexpr const u64 MAX_RENDER_OBJECT_COUNT{ 1024ull };
constexpr const u64 MAX_SKINNED_OBJECT_COUNT{ 2048ull };
constexpr const u64 MAT_FRUSTUM_COUNT{ 128ull };
constexpr const u64 MAX_PRIMITIVE_COUNT{ 1024ull };
constexpr const u64 MAX_MATERIAL_COUNT{ 1024ull };
constexpr const u64 MAX_INDIRECT_DRAW_COUNT{ 1024ull };

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

template<class _Ty>
void writeBuffer(CRenderWorld* self, ERenderBufferIndices type, const stl::vector<_Ty>& data)
{
	auto& buffer = self->getDrawBuffer(type);
	if (buffer->count() < data.size())
		buffer->resize(data.size());

	buffer->map();
	buffer->write(data.data(), data.size());
	buffer->unmap();
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
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eSkinBuffer)) = createDrawBuffer<glm::mat4>("SkinningJoints", EBufferTypeFlagBits::eStorage, MAX_SKINNED_OBJECT_COUNT * 128ull, m_pRenderInterface);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eDirectionalLightBuffer)) = createDrawBuffer<FDirectionalLightCommit>("DirectionalLights", EBufferTypeFlagBits::eStorage, max_directional_light_count, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::ePointLightBuffer)) = createDrawBuffer<FPointLightCommit>("PointLights", EBufferTypeFlagBits::eStorage, max_point_light_count, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eSpotLightBuffer)) = createDrawBuffer<FSpotLightCommit>("SpotLights", EBufferTypeFlagBits::eStorage, max_spot_light_count, m_pRenderInterface);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eCascadedShadowBuffer)) = createDrawBuffer<FCascadedShadowCommit>("CascadedShadows", EBufferTypeFlagBits::eStorage, max_directional_light_shadow_count, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eOmniShadowBuffer)) = createDrawBuffer<FOmniShadowCommit>("OmniShadows", EBufferTypeFlagBits::eStorage, max_point_light_shadow_count, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eSpotShadowBuffer)) = createDrawBuffer<FSpotShadowCommit>("SpotShadows", EBufferTypeFlagBits::eStorage, max_spot_light_shadow_count, m_pRenderInterface);

	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eDrawBatchBuffer)) = createDrawBuffer<FGeometryGPUBatch>("GeometryBatches", EBufferTypeFlagBits::eStorage, MAX_RENDER_OBJECT_COUNT, m_pRenderInterface);
	m_vBuffers.at(utl::enum_cast(ERenderBufferIndices::eFrustumBuffer)) = createDrawBuffer<glm::vec4>("FrustumSides", EBufferTypeFlagBits::eStorage, MAT_FRUSTUM_COUNT * 6ull, m_pRenderInterface);
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

	stl::vector<FDrawIndexedIndirectData> indirect_data{};
	stl::vector<FGeometryGPUBatch> gpu_batches{};
	stl::unordered_map<u64, u64> indirect_command_cache{};

	for (const auto& batch : m_vRawGeometryBatches)
	{
		// Create batch or get already existing
		FGeometryCPUBatch* current_batch{ nullptr };
		if (!m_vGeometryBatches.empty() && m_vGeometryBatches.back().vboID == batch.callInfo.vboID)
			current_batch = &m_vGeometryBatches.back();
		else
		{
			auto& new_batch = m_vGeometryBatches.emplace_back();
			new_batch.vboID = batch.callInfo.vboID;
			new_batch.command.bufferId = getDrawBuffer(ERenderBufferIndices::eDrawCommandBuffer)->handle();
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

			auto& vbo = m_pRenderInterface->getVertexHandler(batch.callInfo.vboID);

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

			indirect_command.aabb.min = batch.range.bounds.min;
			indirect_command.aabb.max = batch.range.bounds.max;

			current_batch->command.count++;
		}

		// Save draw order in all draw array
		gpu_batches.emplace_back(batch.callInfo);
	}

	m_vRawGeometryBatches.clear();
	m_stats.drawCommandCount = indirect_data.size();

	writeBuffer(this, ERenderBufferIndices::eDrawBatchBuffer, gpu_batches);

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

	writeBuffer(this, ERenderBufferIndices::eMaterialBuffer, m_vMaterials);
	writeBuffer(this, ERenderBufferIndices::eNodeBuffer, m_vNodes);
	writeBuffer(this, ERenderBufferIndices::eSkinBuffer, m_vSkins);

	writeBuffer(this, ERenderBufferIndices::eDirectionalLightBuffer, m_vDirectionalLights);
	writeBuffer(this, ERenderBufferIndices::ePointLightBuffer, m_vPointLights);
	writeBuffer(this, ERenderBufferIndices::eSpotLightBuffer, m_vSpotLights);

	writeBuffer(this, ERenderBufferIndices::eCascadedShadowBuffer, m_vCascadedShadows);
	writeBuffer(this, ERenderBufferIndices::eOmniShadowBuffer, m_vOmniShadows);
	writeBuffer(this, ERenderBufferIndices::eSpotShadowBuffer, m_vSpotShadows);

	writeBuffer(this, ERenderBufferIndices::eDrawCommandBuffer, indirect_data);

	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& texture_remap_data = render_context->getGlobalTextureRemap();
	writeBuffer(this, ERenderBufferIndices::eTextureRemapBuffer, texture_remap_data);
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
		m_vNodes.resize((node_id + 1ull));

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