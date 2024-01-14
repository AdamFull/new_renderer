#include "RenderWorld.h"

#include "BufferObject.h"

using namespace engine::render;

constexpr const size_t MAX_RENDER_OBJECT_COUNT{ 16384ull };
constexpr const size_t MAX_PRIMITIVE_COUNT{ 512ull };
constexpr const size_t MAX_MATERIAL_COUNT{ 512ull };

template<class _Ty>
std::unique_ptr<IBufferObject> createDrawBuffer(const std::string& name, uint32_t type, size_t count, IRenderInterface* render_interface)
{
	FBufferCreateInfo buffer_ci{};
	buffer_ci.flags = type;
	buffer_ci.size = sizeof(_Ty);
	buffer_ci.count = count;

	auto new_buffer = std::make_unique<CBufferObject>(render_interface);
	new_buffer->create(name, buffer_ci);
	return new_buffer;
}

CRenderWorld::CRenderWorld(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CRenderWorld::create()
{
	m_vBuffers.emplace_back(createDrawBuffer<FSceneNode>("NodeStorage", BUFFER_TYPE_STORAGE_FLAG_BIT, MAX_PRIMITIVE_COUNT, m_pRenderInterface));
	m_vBuffers.emplace_back(createDrawBuffer<FMaterial>("MaterialStorage", BUFFER_TYPE_STORAGE_FLAG_BIT, MAX_MATERIAL_COUNT, m_pRenderInterface));
	m_vBuffers.emplace_back(createDrawBuffer<glm::mat4>("SkinningJoints", BUFFER_TYPE_STORAGE_FLAG_BIT, MAX_PRIMITIVE_COUNT * 128ull, m_pRenderInterface));

	m_vBuffers.emplace_back(createDrawBuffer<FDirectionalLightCommit>("DirectionalLights", BUFFER_TYPE_STORAGE_FLAG_BIT, max_directional_light_count, m_pRenderInterface));
	m_vBuffers.emplace_back(createDrawBuffer<FPointLightCommit>("PointLights", BUFFER_TYPE_STORAGE_FLAG_BIT, max_point_light_count, m_pRenderInterface));
	m_vBuffers.emplace_back(createDrawBuffer<FSpotLightCommit>("SpotLights", BUFFER_TYPE_STORAGE_FLAG_BIT, max_spot_light_count, m_pRenderInterface));

	m_vBuffers.emplace_back(createDrawBuffer<FCascadedShadowCommit>("CascadedShadows", BUFFER_TYPE_STORAGE_FLAG_BIT, max_directional_light_shadow_count, m_pRenderInterface));
	m_vBuffers.emplace_back(createDrawBuffer<FOmniShadowCommit>("OmniShadows", BUFFER_TYPE_STORAGE_FLAG_BIT, max_point_light_shadow_count, m_pRenderInterface));
	m_vBuffers.emplace_back(createDrawBuffer<FSpotShadowCommit>("SpotShadows", BUFFER_TYPE_STORAGE_FLAG_BIT, max_spot_light_shadow_count, m_pRenderInterface));

	m_vBuffers.emplace_back(createDrawBuffer<FGeometryGPUBatch>("GeometryBatches", BUFFER_TYPE_STORAGE_FLAG_BIT, MAX_RENDER_OBJECT_COUNT, m_pRenderInterface));
	m_vBuffers.emplace_back(createDrawBuffer<FGeometryGPUCullInfo>("GeometryCullingInfo", BUFFER_TYPE_STORAGE_FLAG_BIT, MAX_RENDER_OBJECT_COUNT, m_pRenderInterface));
	m_vBuffers.emplace_back(createDrawBuffer<glm::vec4>("FrustumSides", BUFFER_TYPE_STORAGE_FLAG_BIT, MAX_PRIMITIVE_COUNT * 6ull, m_pRenderInterface));
	m_vBuffers.emplace_back(createDrawBuffer<FDrawIndexedIndirectData>("IndexedDrawIndirectBuffer", BUFFER_TYPE_INDIRECT_FLAG_BIT, max_indirect_draw_count, m_pRenderInterface));
}

size_t CRenderWorld::registerFrustum(uint32_t type)
{
	size_t frustum_id = m_vSceneFrustums.size();
	m_vSceneFrustums.emplace_back();

	auto& culling_info = m_vCullingInfo.emplace_back();
	culling_info.cullerID = frustum_id;
	culling_info.type = type;

	return frustum_id;
}

void CRenderWorld::updateFrustum(size_t frustum_id, const glm::mat4& view, const glm::mat4& projection)
{
	auto& frustum = m_vSceneFrustums.at(frustum_id);
	frustum.update(view, projection);
}

void CRenderWorld::addDrawGeometry(const std::vector<FGeometryRawBatch>& new_geometry)
{
	m_vRawGeometryBatches.insert(m_vRawGeometryBatches.end(), new_geometry.begin(), new_geometry.end());

	// Sort drawings by vbo, material and node
	std::sort(m_vRawGeometryBatches.begin(), m_vRawGeometryBatches.end(), [](const FGeometryRawBatch& a, const FGeometryRawBatch& b)
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

void CRenderWorld::render(float dt)
{
	// Clear old frame data
	m_vGeometryBatches.clear();

	for (auto& [type, range] : m_mBatchRanges)
		range.clear();

	std::vector<FGeometryGPUCullInfo> gpu_culling_infos{};
	std::vector<FDrawIndexedIndirectData> indirect_data{};
	std::vector<FGeometryGPUBatch> gpu_batches{};
	std::vector<uint32_t> counted{};

	size_t command_offset{ 0ull };
	size_t batch_offset{ 0ull };

	for (const auto& cull_info : m_vCullingInfo)
	{
		size_t current_material{ ~0ull };
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
				gpu_batches.back().instanceID == batch.callInfo.instanceID &&
				current_material + 1 == batch.callInfo.materialID)
			{
				counted.back()++;
				current_material = batch.callInfo.materialID;
			}
			else
			{
				auto& new_gpu_batch = gpu_batches.emplace_back();
				new_gpu_batch = batch.callInfo;
				counted.emplace_back(1ull);

				current_material = batch.callInfo.materialID;
			}
		}

		auto& batch_ranges = m_mBatchRanges[cull_info.type];
		auto& batch_range = batch_ranges.emplace_back();
		batch_range.offset = m_vGeometryBatches.size();

		// Prepare cpu data
		for (size_t idx = batch_offset; idx < gpu_batches.size(); ++idx)
		{
			auto& gpu_batch = gpu_batches.at(idx);

			auto& new_cpu_batch = m_vGeometryBatches.emplace_back();
			new_cpu_batch.vboID = gpu_batch.vboID;
			new_cpu_batch.command.bufferId = m_vBuffers.at(draw_command_buffer_index)->handle();
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

	auto& batch_buffer = m_vBuffers.at(draw_batch_buffer_index);
	batch_buffer->map();
	batch_buffer->write(gpu_batches.data(), gpu_batches.size());
	batch_buffer->unmap();

	auto& culling_buffer = m_vBuffers.at(culling_info_buffer_index);
	culling_buffer->map();
	culling_buffer->write(gpu_culling_infos.data(), gpu_culling_infos.size());
	culling_buffer->unmap();

	// Copy frustum sides to gpu buffer
	{
		auto& frustum_buffer = m_vBuffers.at(frustum_buffer_index);
		frustum_buffer->map();

		size_t offset{ 0ull };
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

	auto& material_buffer = m_vBuffers.at(material_buffer_index);
	material_buffer->map();
	material_buffer->write(m_vMaterials.data(), m_vMaterials.size());
	material_buffer->unmap();

	auto& node_buffer = m_vBuffers.at(node_buffer_index);
	node_buffer->map();
	node_buffer->write(m_vNodes.data(), m_vNodes.size());
	node_buffer->unmap();

	auto& skin_buffer = m_vBuffers.at(skin_buffer_index);
	skin_buffer->map();
	skin_buffer->write(m_vSkins.data(), m_vSkins.size());
	skin_buffer->unmap();

	auto& directional_light_buffer = m_vBuffers.at(directional_light_buffer_index);
	directional_light_buffer->map();
	directional_light_buffer->write(m_vDirectionalLights.data(), m_vDirectionalLights.size());
	directional_light_buffer->unmap();

	auto& point_light_buffer = m_vBuffers.at(point_light_buffer_index);
	point_light_buffer->map();
	point_light_buffer->write(m_vPointLights.data(), m_vPointLights.size());
	point_light_buffer->unmap();

	auto& spot_light_buffer = m_vBuffers.at(spot_light_buffer_index);
	spot_light_buffer->map();
	spot_light_buffer->write(m_vSpotLights.data(), m_vSpotLights.size());
	spot_light_buffer->unmap();

	auto& cascaded_shadow_buffer = m_vBuffers.at(cascaded_shadow_buffer_index);
	cascaded_shadow_buffer->map();
	cascaded_shadow_buffer->write(m_vCascadedShadows.data(), m_vCascadedShadows.size());
	cascaded_shadow_buffer->unmap();

	auto& omni_shadow_buffer = m_vBuffers.at(omni_shadow_buffer_index);
	omni_shadow_buffer->map();
	omni_shadow_buffer->write(m_vOmniShadows.data(), m_vOmniShadows.size());
	omni_shadow_buffer->unmap();

	auto& spot_shadow_buffer = m_vBuffers.at(spot_shadow_buffer_index);
	spot_shadow_buffer->map();
	spot_shadow_buffer->write(m_vSpotShadows.data(), m_vSpotShadows.size());
	spot_shadow_buffer->unmap();

	auto& draw_command_buffer = m_vBuffers.at(draw_command_buffer_index);
	draw_command_buffer->map();
	draw_command_buffer->write(indirect_data.data(), indirect_data.size());
	draw_command_buffer->unmap();
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

void CRenderWorld::updateNode(uint32_t node_id, const FSceneNode& node)
{
	if (m_vNodes.size() < node_id + 1ull)
		m_vNodes.resize((node_id + 1ull) * 2ull);

	m_vNodes.at(node_id) = node;
}

void CRenderWorld::updateSkin(const std::array<glm::mat4, 128ull>& matrices)
{
	if (m_vSkins.size() < (m_stats.skinCount * 128ull) + 1ull)
		m_vSkins.resize(((m_stats.skinCount * 128ull) + 1ull) * 2ull);

	auto skin_begin = m_vSkins.begin() + (m_stats.skinCount * 128ull);
	for (size_t idx = 0ull; idx < 128ull; ++idx)
		m_vSkins.at(m_stats.skinCount * 128ull + idx) = matrices.at(idx);

	m_stats.skinCount++;
}

void CRenderWorld::addMaterial(const FMaterial& material)
{
	m_vMaterials.emplace_back(material);
}

void CRenderWorld::addMaterials(const std::vector<FMaterial>& materials)
{
	m_vMaterials.insert(m_vMaterials.end(), materials.begin(), materials.end());
}

const std::vector<std::unique_ptr<IBufferObject>>& CRenderWorld::getDrawBuffers()
{
	return m_vBuffers;
}

const std::unordered_map<uint32_t, std::vector<FGeometryBatchRange>>& CRenderWorld::getAllGeometryRanges() const
{
	return m_mBatchRanges;
}

const std::vector<FGeometryBatchRange>& CRenderWorld::getGeometryRanges(uint32_t type) const
{
	return m_mBatchRanges[type];
}

const FGeometryCPUBatch& CRenderWorld::getGeometryBatch(size_t index) const
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