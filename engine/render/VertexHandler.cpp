#include "VertexHandler.h"

#include "GPUVector.h"

#include <logger/logger.h>
#include <meshoptimizer.h>
#include <glm/gtc/type_ptr.hpp>

using namespace engine::render;

// For debugging
//struct FVertex
//{
//	alignas(16) glm::vec3 pos{};
//	alignas(8) glm::vec2 uv{};
//	alignas(16) glm::vec3 norm{};
//	alignas(16) glm::vec4 tan{};
//	alignas(4) glm::u8vec4 joints{};
//	alignas(16) glm::vec4 weights{};
//};

u64 align_std140(u64 size)
{
	constexpr const u64 maxalignment{ 0ull };
	if (size <= 4ull)
		return 4ull;
	else if (size <= 8ull)
		return 8ull;
	else if (size <= 16ull)
		return 16ull;
	
	return (size + maxalignment - 1ull) & ~(maxalignment - 1ull);
}

EApiDataFormat get_attribute_format_by_type(EVertexAttributeType type, u64 size)
{
	if (size == 4ull)
	{
		switch (type)
		{
		case EVertexAttributeType::eJoint: return EApiDataFormat::eRGBA8_UNORM; break;
		}
	}
	else if (size == 8ull)
	{
		switch (type)
		{
		case EVertexAttributeType::eTexCoord: return EApiDataFormat::eRG32_SFLOAT; break;
		case EVertexAttributeType::eJoint: return EApiDataFormat::eRGBA16_UNORM; break;
		}
	}
	else if (size == 12ull)
	{
		switch (type)
		{
		case EVertexAttributeType::ePosition:
		case EVertexAttributeType::eColor: 
		case EVertexAttributeType::eNormal: 
		case EVertexAttributeType::eTangent: return EApiDataFormat::eRGBA32_SFLOAT; break;
		}
	}
	else if (size == 16ull)
	{
		switch (type)
		{
		case EVertexAttributeType::ePosition:
		case EVertexAttributeType::eColor:
		case EVertexAttributeType::eTangent:
		case EVertexAttributeType::eWeight: return EApiDataFormat::eRGBA32_SFLOAT; break;
		}
	}

	return EApiDataFormat::eUnknown;
}
// HAS_TEXCOORD, HAS_NORMAL, HAS_SKIN, COLOR_VEC3, COLOR_VEC4, TANGENT_VEC3, TANGENT_VEC4, JOINTS_8BIT, JOINTS_16BIT
stl::string get_attribute_definition(EVertexAttributeType type, u64 size)
{
	if (size == 4ull)
	{
		switch (type)
		{
		case EVertexAttributeType::eJoint: return "JOINTS_8BIT"; break;
		}
	}
	else if (size == 8ull)
	{
		switch (type)
		{
		case EVertexAttributeType::eTexCoord: return "HAS_TEXCOORD"; break;
		case EVertexAttributeType::eJoint: return "JOINTS_16BIT"; break;
		}
	}
	else if (size == 12ull)
	{
		switch (type)
		{
		case EVertexAttributeType::eColor: return "COLOR_VEC3"; break;
		case EVertexAttributeType::eNormal: return "HAS_NORMAL"; break;
		case EVertexAttributeType::eTangent: return "TANGENT_VEC3"; break;
		}
	}
	else if (size == 16ull)
	{
		switch (type)
		{
		case EVertexAttributeType::eColor: return "COLOR_VEC4"; break;
		case EVertexAttributeType::eTangent: return "TANGENT_VEC4"; break;
		case EVertexAttributeType::eWeight: return "HAS_SKIN"; break;
		}
	}

	return "";
}

stl::string get_index_definition(u64 size)
{
	if (size == 1ull)
		return "INDEX_8BIT";
	else if (size == 2ull)
		return "INDEX_16BIT";

	return "";
}

CVertexHandler::CVertexHandler(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
	m_vGeometryBuffers.resize(utl::enum_cast(EGeometryBufferIndices::eLastIndex));

	for(auto& buffer : m_vGeometryBuffers)
		buffer = std::make_unique<CGPUVector>(render_interface);
}

// TODO: destroy buffers
CVertexHandler::~CVertexHandler()
{
	auto& render_context = m_pRenderInterface->getRenderContext();
	render_context->destroyBuffer(m_vertexBuffer.buffer);
	render_context->destroyBuffer(m_indexBuffer.buffer);
}

// NOTE: current implementation work only with interleaved vertex
void CVertexHandler::create(const FVertexHandlerCreateInfo& create_info)
{
	stl::unordered_set<stl::string> definitions{};

	// Initialize attribute descriptions
	auto& render_context = m_pRenderInterface->getRenderContext();
	for (const auto& desc : create_info.descriptions)
	{
		//if (desc.type == EVertexAttributeType::eJoint)
		//	skinned = true;

		auto& attrib = m_declaration.attributeDescriptions.emplace_back();
		attrib.type = desc.type;
		attrib.binding = 0u;
		attrib.location = utl::cast<u32>(desc.type);

		auto format = get_attribute_format_by_type(desc.type, desc.size);
		attrib.format = render_context->getApiFormat(format);
		attrib.offset = m_vertexBuffer.elementSize;

		if(desc.type != EVertexAttributeType::ePosition)
			definitions.emplace(get_attribute_definition(desc.type, desc.size));

		m_vertexBuffer.elementSize += desc.size;
	}

	auto& desc = m_declaration.bindingDescriptions.emplace_back();
	desc.binding = 0u;
	desc.stride = m_vertexBuffer.elementSize;

	m_indexBuffer.elementSize = create_info.indexSize;

	if (m_indexBuffer.elementSize != 4ull)
		definitions.emplace(get_index_definition(m_indexBuffer.elementSize));

	// Initialize attribute definitions and hash
	stl::stringstream ss{};
	for (const auto& definition : definitions)
		ss << ":" << definition;
	m_definitions = ss.str();

	m_attributeHash = utl::utils::fnv1a_64_hash(m_definitions.c_str());

	log_info("[Vertex Handler] Created vertex handler: {}:{}.", create_info.name, m_definitions);

	m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eMeshletBuffer))->create<FMeshlet>("MeshletBuffer" + m_definitions, EBufferTypeFlagBits::eStorage);
	m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eVertexRemapBuffer))->create<u32>("MeshletVerticesBuffer" + m_definitions, EBufferTypeFlagBits::eStorage);
	m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eBoundsBuffer))->create<FGeometryDrawBounds>("MeshletBoundsBuffer" + m_definitions, EBufferTypeFlagBits::eStorage);
	m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eTriangleBuffer))->create<FTriangle>("MeshletTriangleBuffer" + m_definitions, EBufferTypeFlagBits::eStorage);
	m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eLodBuffer))->create<FMeshLod>("MeshLodBuffer" + m_definitions, EBufferTypeFlagBits::eStorage);

	return;
}

void CVertexHandler::allocate()
{
	auto& render_context = m_pRenderInterface->getRenderContext();

	// Create vertex buffer
	FBufferCreateInfo buffer_ci{};
	buffer_ci.flags = EBufferTypeFlagBits::eVertex;
	buffer_ci.size = m_vertexBuffer.elementSize;
	buffer_ci.count = m_vertexBuffer.elementCount;
	buffer_ci.upload.data = m_vertexBuffer.data;
	m_vertexBuffer.buffer = render_context->createBuffer(stl::format("SCENE_VERTICES{}", m_definitions), buffer_ci);

	// Create index buffer
	buffer_ci.flags = EBufferTypeFlagBits::eIndex;
	buffer_ci.size = m_indexBuffer.elementSize;
	buffer_ci.count = m_indexBuffer.elementCount;
	buffer_ci.upload.data = m_indexBuffer.data;
	m_indexBuffer.buffer = render_context->createBuffer(stl::format("SCENE_INDICES{}", m_definitions), buffer_ci);
}

// TODO: i can create custom bindings support here
void CVertexHandler::bind(EQueueTypeFlagBits queue)
{
	if (m_vertexBuffer.elementCount)
	{
		auto& render_context = m_pRenderInterface->getRenderContext();
		{
			FVertexBufferBindingInfo binding_info{};
			binding_info.buffers.emplace_back(m_vertexBuffer.buffer);
			binding_info.offsets.emplace_back(0ull);
			render_context->bindVertexBuffers(binding_info, queue);
		}

		if (m_indexBuffer.elementCount)
		{
			FIndexBufferBindingInfo binding_info{};
			binding_info.bufferId = m_indexBuffer.buffer;
			binding_info.indexSize = m_indexBuffer.elementSize;
			render_context->bindIndexBuffer(binding_info, queue);
		}
	}
}

const FVertexRange& CVertexHandler::addMeshData(const stl::vector<FVertexAttributePointer>& attributes_data, const FVertexAttributePointer& indices)
{
	u64 input_hash = utl::utils::murmur_hash_64(indices.data, indices.stride * indices.count, 0);
	for (const auto& attribute : attributes_data)
		input_hash = utl::utils::murmur_hash_64(attribute.data, attribute.stride * attribute.count, input_hash);

	if (auto found = m_cachedRanges.find(input_hash); found != m_cachedRanges.end())
		return found->second;

	auto& meshlet_buffer = m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eMeshletBuffer));
	auto& vertices_buffer = m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eVertexRemapBuffer));
	auto& triangle_buffer = m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eTriangleBuffer));
	auto& bounds_buffer = m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eBoundsBuffer));
	auto& lod_buffer = m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eLodBuffer));

	FMeshLod lod{};
	lod.meshletOffset = meshlet_buffer->size();
	lod.meshletVertexOffset = vertices_buffer->size();
	lod.meshletTriangleOffset = triangle_buffer->size();
	lod.meshletBoundsOffset = bounds_buffer->size();
	lod.meshletCount = addMeshlets(attributes_data, indices);

	auto& new_range = m_cachedRanges[input_hash];
	new_range.firstLod = lod_buffer->size();
	new_range.lodCount = 1;
	new_range.firstIndex = m_indexBuffer.elementCount;
	new_range.indexCount = addIndices(indices);
	new_range.firstVertex = m_vertexBuffer.elementCount;
	new_range.vertexCount = addVertices(attributes_data);

	lod_buffer->emplace_back(lod);

	return new_range;
}

u64 CVertexHandler::addVertices(const stl::vector<FVertexAttributePointer>& attributes_data)
{
	auto vertex_count = attributes_data.front().count;
	for (u64 v = 0ull; v < vertex_count; ++v)
	{
		stl::vector<uint8_t> temp(m_vertexBuffer.elementSize);

		u64 offset{ 0ull };
		for (const auto& attrib : attributes_data)
		{
			stl::memcpy(temp.data() + offset, &attrib.data[v * attrib.stride], attrib.stride);
			offset += attrib.stride;
		}

		m_vertexBuffer.data.insert(m_vertexBuffer.data.end(), temp.begin(), temp.end());
	}

	m_vertexBuffer.elementCount += vertex_count;
	return vertex_count;
}

u64 CVertexHandler::addIndices(const FVertexAttributePointer& indices)
{
	m_indexBuffer.data.insert(m_indexBuffer.data.end(), indices.data, indices.data + (indices.count * m_indexBuffer.elementSize));
	m_indexBuffer.elementCount += indices.count;
	return indices.count;
}

u64 CVertexHandler::addMeshlets(const stl::vector<FVertexAttributePointer>& attributes, const FVertexAttributePointer& indices)
{
	// Find positions attribute
	const float* positions{ nullptr };
	auto found_position_attrib = std::find_if(attributes.begin(), attributes.end(), [](const FVertexAttributePointer& attribute) { return attribute.type == EVertexAttributeType::ePosition; });
	if (found_position_attrib != attributes.end())
		positions = (const float*)found_position_attrib->data;

	// Convert indices to u32 format
	stl::vector<u32> indices_tmp{};
	// convert to 32 bit uint
	for (u64 idx = 0ull; idx < indices.count; ++idx)
	{
		if (indices.stride == 2)
		{
			const u16* indices16 = (const u16*)indices.data;
			indices_tmp.emplace_back(indices16[idx]);
		}
		else
		{
			const u32* indices32 = (const u32*)indices.data;
			indices_tmp.emplace_back(indices32[idx]);
		}
	}

	// Start generation meshlets
	auto max_meshlets = meshopt_buildMeshletsBound(indices.count, kMaxVerticesPerMeshlet, kMaxTrianglesPerMeshlet);

	std::vector<FMeshlet> temp_meshlets(max_meshlets);
	std::vector<FGeometryDrawBounds> temp_bounds{};
	std::vector<FTriangle> temp_triangles{};
	std::vector<u32> temp_vertices(max_meshlets * kMaxVerticesPerMeshlet);

	std::vector<meshopt_Meshlet> raw_meshlets(max_meshlets);
	std::vector<u8> raw_meshlet_triangles(max_meshlets * kMaxTrianglesPerMeshlet * 3u);

	size_t meshlet_count = meshopt_buildMeshlets(
		&raw_meshlets[0],
		&temp_vertices[0],
		&raw_meshlet_triangles[0],
		&indices_tmp[0],
		indices.count,
		positions,
		found_position_attrib->count,
		found_position_attrib->stride,
		kMaxVerticesPerMeshlet,
		kMaxTrianglesPerMeshlet,
		/*cone_weight*/ 0);

	meshopt_Meshlet& last_meshlet = raw_meshlets[meshlet_count - 1];
	raw_meshlet_triangles.resize(last_meshlet.triangle_offset + ((last_meshlet.triangle_count * 3 + 3) & ~3));
	raw_meshlets.resize(meshlet_count);

	temp_vertices.resize(last_meshlet.vertex_offset + size_t(last_meshlet.vertex_count));
	temp_meshlets.resize(meshlet_count);
	temp_bounds.resize(meshlet_count);
	temp_triangles.resize(raw_meshlet_triangles.size() / 3);

	auto& triangle_buffer = m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eTriangleBuffer));
	auto& vertices_buffer = m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eVertexRemapBuffer));

	u32 triangle_offset{ 0u };
	for (u32 idx = 0u; idx < meshlet_count; ++idx)
	{
		const auto& raw_meshlet = raw_meshlets[idx];

		auto& bounds = temp_bounds[idx];
		for (u32 tri = 0u; tri < raw_meshlet.triangle_count * 3u; ++tri)
		{
			u32 tri_idx =temp_vertices[raw_meshlet.vertex_offset + raw_meshlet_triangles[raw_meshlet.triangle_offset + tri]];
			glm::vec3 p = glm::make_vec3(positions + tri_idx);
			bounds.max = (glm::max)(bounds.max, p);
			bounds.min = (glm::min)(bounds.min, p);
		}

		u8* triangles_indices = raw_meshlet_triangles.data() + raw_meshlet.triangle_offset;
		for (u32 tri_idx = 0; tri_idx < raw_meshlet.triangle_count; ++tri_idx)
		{
			auto& tri = temp_triangles[tri_idx + triangle_offset];
			tri.V0 = *triangles_indices++;
			tri.V1 = *triangles_indices++;
			tri.V2 = *triangles_indices++;
		}

		auto& meshlet = temp_meshlets[idx];
		meshlet.triangleCount = raw_meshlet.triangle_count;
		meshlet.triangleOffset = triangle_offset + triangle_buffer->size();
		meshlet.vertexCount = raw_meshlet.vertex_count;
		meshlet.vertexOffset = raw_meshlet.vertex_offset + vertices_buffer->size(); // set from first vertex

		triangle_offset += raw_meshlet.triangle_count;
	}
	temp_triangles.resize(triangle_offset);

	auto& meshlet_buffer = m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eMeshletBuffer));
	meshlet_buffer->copy(temp_meshlets);

	auto& bounds_buffer = m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eBoundsBuffer));
	bounds_buffer->copy(temp_bounds);

	vertices_buffer->copy(temp_vertices);
	triangle_buffer->copy(temp_triangles);

	return meshlet_count;
}

u64 CVertexHandler::getLastVertex()
{
	return m_vertexBuffer.elementCount;
}

u64 CVertexHandler::getLastIndex()
{
	return m_indexBuffer.elementCount;
}

const FVertexDeclarationInfo& CVertexHandler::getVertexDeclaration() const
{
	return m_declaration;
}

u64 CVertexHandler::getHash() const
{
	return m_attributeHash;
}

const stl::string& CVertexHandler::getDefinitions() const
{
	return m_definitions;
}

const std::unique_ptr<IGPUVector>& CVertexHandler::getGeometryBuffer(EGeometryBufferIndices buffer_index)
{
	return m_vGeometryBuffers.at(utl::enum_cast(buffer_index));
}

u64 CVertexHandler::getVertexHandle() const
{
	return m_vertexBuffer.buffer;
}

u64 CVertexHandler::getIndexHandle() const
{
	return m_indexBuffer.buffer;
}