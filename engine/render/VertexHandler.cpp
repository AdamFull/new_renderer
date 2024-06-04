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

utl::vector<u32> convert_to_u32_indices(const FVertexAttributePointer& indices)
{
	// Convert indices to u32 format
	utl::vector<u32> indices_tmp{};
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

	return indices_tmp;
}

utl::vector<u16> convert_u32_indices_to_u16(const utl::vector<u32>& indices)
{
	utl::vector<u16> new_indices{};

	new_indices.reserve(indices.size());
	for (const u32& index : indices)
		new_indices.emplace_back(utl::cast<u16>(index));

	return new_indices;
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
		case EVertexAttributeType::ePosition:
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
		case EVertexAttributeType::eTangent: return EApiDataFormat::eRGB32_SFLOAT; break;
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
std::string get_attribute_definition(EVertexAttributeType type, u64 size)
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

std::string get_index_definition(u64 size)
{
	if (size == 2ull)
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
	auto& render_context = m_pRenderInterface->GetRenderContext();
}

// NOTE: current implementation work only with interleaved vertex
void CVertexHandler::Create(const FVertexHandlerCreateInfo& create_info)
{
	std::unordered_set<std::string> definitions{};

	m_flags = create_info.flags;

	// Initialize attribute descriptions
	u64 vertex_size{ 0ull };
	auto& render_context = m_pRenderInterface->GetRenderContext();
	for (const auto& desc : create_info.descriptions)
	{
		//if (desc.type == EVertexAttributeType::eJoint)
		//	skinned = true;

		auto& attrib = m_declaration.attributeDescriptions.emplace_back();
		attrib.type = desc.type;
		attrib.binding = 0u;
		attrib.location = utl::cast<u32>(desc.type);

		auto format = get_attribute_format_by_type(desc.type, desc.size);
		attrib.format = render_context->GetApiFormat(format);
		attrib.offset = vertex_size;

		if(desc.type != EVertexAttributeType::ePosition)
			definitions.emplace(get_attribute_definition(desc.type, desc.size));

		vertex_size += desc.size;
	}

	auto& desc = m_declaration.bindingDescriptions.emplace_back();
	desc.binding = 0u;
	desc.stride = vertex_size;

	if (create_info.indexSize != 4ull)
		definitions.emplace(get_index_definition(create_info.indexSize));

	// Initialize attribute definitions and hash
	std::stringstream ss{};
	for (const auto& definition : definitions)
		ss << ":" << definition;
	m_definitions = ss.str();

	m_attributeHash = utl::fnv1a_64_hash_cstr(m_definitions.c_str());

	log_info("[Vertex Handler] Created vertex handler: {}:{}.", create_info.name, m_definitions);

	m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eVertexBuffer))->CreateRaw("SCENE_VERTICES" + m_definitions, vertex_size, 16ull, EBufferUsageFlagBits::eVertexBuffer);
	m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eIndexBuffer))->CreateRaw("SCENE_INDICES" + m_definitions, create_info.indexSize, 16ull, EBufferUsageFlagBits::eIndexBuffer);
	m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eLodBuffer))->Create<FMeshLod>("MeshLodBuffer" + m_definitions, 16ull, EBufferUsageFlagBits::eStorageBuffer);
	
	if (create_info.flags & EVertexHandlerFlagBits::eGenerateMeshlets)
	{
		m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eMeshletBuffer))->Create<FMeshlet>("MeshletBuffer" + m_definitions, 16ull, EBufferUsageFlagBits::eStorageBuffer);
		m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eVertexRemapBuffer))->Create<u32>("MeshletVerticesBuffer" + m_definitions, 16ull, EBufferUsageFlagBits::eStorageBuffer);
		m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eBoundsBuffer))->Create<FGeometryDrawBounds>("MeshletBoundsBuffer" + m_definitions, 16ull, EBufferUsageFlagBits::eStorageBuffer);
		m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eTriangleBuffer))->Create<FTriangle>("MeshletTriangleBuffer" + m_definitions, 16ull, EBufferUsageFlagBits::eStorageBuffer);
	}

	return;
}

void CVertexHandler::Allocate()
{
	auto& render_context = m_pRenderInterface->GetRenderContext();

	for (auto& buffer : m_vGeometryBuffers)
		buffer->Upload();
}

// TODO: i can Create custom bindings support here
void CVertexHandler::Bind(EQueueTypeFlagBits queue)
{
	auto& vertex_buffer = GetGeometryBuffer(EGeometryBufferIndices::eVertexBuffer);
	auto& index_buffer = GetGeometryBuffer(EGeometryBufferIndices::eIndexBuffer);

	if (index_buffer->Count())
	{
		auto& render_context = m_pRenderInterface->GetRenderContext();
		{
			FVertexBufferBindingInfo binding_info{};
			binding_info.buffers.emplace_back(vertex_buffer->Buffer()->Handle());
			binding_info.offsets.emplace_back(0ull);
			render_context->BindVertexBuffers(binding_info, queue);
		}

		if (index_buffer->Count())
		{
			FIndexBufferBindingInfo binding_info{};
			binding_info.bufferId = index_buffer->Buffer()->Handle();
			binding_info.indexSize = index_buffer->Stride();
			render_context->BindIndexBuffer(binding_info, queue);
		}
	}
}

const FVertexRange& CVertexHandler::AddMeshData(const utl::vector<FVertexAttributePointer>& attributes_data, const FVertexAttributePointer& indices)
{
	u64 input_hash = utl::murmur_hash_64(indices.data, indices.stride * indices.count, 0);
	for (const auto& attribute : attributes_data)
		input_hash = utl::murmur_hash_64(attribute.data, attribute.stride * attribute.count, input_hash);

	if (auto found = m_cachedRanges.find(input_hash); found != m_cachedRanges.end())
		return found->second;

	auto position_attrib = std::find_if(attributes_data.begin(), attributes_data.end(), [](const FVertexAttributePointer& attribute) { return attribute.type == EVertexAttributeType::ePosition; });
	if (position_attrib == attributes_data.end())
		log_error("[Vertex Handler] Invalid mesh type: No vertex position found. Vertex position is required.");

	auto& lod_buffer = m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eLodBuffer));

	// Calculate number of lods
	f32 triangle_count = utl::cast<f32>(indices.count / 3u);
	u32 lod_count = utl::cast<u32>(glm::floor(glm::log2(triangle_count))) / 2u;

	lod_count = (m_flags & EVertexHandlerFlagBits::eGenerateLods) ? lod_count : 1u;

	// Create new vertex range
	auto& new_range = m_cachedRanges[input_hash];
	new_range.firstLod = lod_buffer->Count();
	new_range.lodCount = lod_count;

	auto orig_indices = convert_to_u32_indices(indices);
	const f32* positions = reinterpret_cast<const f32*>(position_attrib->data);

	//meshopt_optimizeVertexFetch((void*)position_attrib->data, orig_indices.data(), orig_indices.size(), position_attrib->data, position_attrib->count, position_attrib->stride);
	meshopt_optimizeVertexCache(orig_indices.data(), orig_indices.data(), orig_indices.size(), position_attrib->count);

	u32 last_lod_count{ 0ull };
	for (u64 lod_idx = 0ull; lod_idx < lod_count; ++lod_idx)
	{
		utl::vector<u32> u32_indices{};
		utl::vector<u16> u16_indices{};

		FVertexAttributePointer new_index_ptr{};
		new_index_ptr.type = indices.type;
		new_index_ptr.stride = indices.stride;

		// Create new index buffer for current lod
		if (lod_idx == 0ull)
		{
			u32_indices = orig_indices;

			if (indices.stride == 2ull)
			{
				u16_indices = convert_u32_indices_to_u16(u32_indices);
				new_index_ptr.data = reinterpret_cast<const u8*>(u16_indices.data());
			}
			else
				new_index_ptr.data = reinterpret_cast<const u8*>(u32_indices.data());

			new_index_ptr.count = orig_indices.size();
		}
		else
		{
			const f32 target_error = 1e-2f;
			bool lock_borders = false;
			u32 options = lock_borders ? meshopt_SimplifyLockBorder : 0;

			//f32 lod_quality = 1.f / utl::cast<f32>(1 << lod_idx);
			f32 lod_quality = 1.f - (utl::cast<f32>(lod_idx) / utl::cast<f32>(lod_count));
			lod_quality *= lod_quality;

			u64 target_index_count = utl::cast<u64>(utl::cast<f64>(orig_indices.size() / 3ull) * lod_quality) * 3ull;
			u32_indices.resize(orig_indices.size());

			float lod_error = 0.f;
			u32_indices.resize(meshopt_simplify<u32>(u32_indices.data(), orig_indices.data(), orig_indices.size(), positions, position_attrib->count, position_attrib->stride, target_index_count, target_error, options, &lod_error));

			if (last_lod_count == u32_indices.size())
			{
				--new_range.lodCount;
				continue;
			}

			last_lod_count = u32_indices.size();

			// Converting back
			if (indices.stride == 2ull)
			{
				u16_indices = convert_u32_indices_to_u16(u32_indices);
				new_index_ptr.data = reinterpret_cast<const u8*>(u16_indices.data());
			}
			else
				new_index_ptr.data = reinterpret_cast<const u8*>(u32_indices.data());

			new_index_ptr.count = u32_indices.size();
		}

		FMeshLod lod{};

		if (m_flags & EVertexHandlerFlagBits::eGenerateMeshlets)
		{
			auto& meshlet_buffer = m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eMeshletBuffer));
			auto& vertices_buffer = m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eVertexRemapBuffer));
			auto& triangle_buffer = m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eTriangleBuffer));
			auto& bounds_buffer = m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eBoundsBuffer));

			//lod.meshletOffset = meshlet_buffer->count();
			//lod.meshletVertexOffset = vertices_buffer->count();
			//lod.meshletTriangleOffset = triangle_buffer->count();
			//lod.meshletBoundsOffset = bounds_buffer->count();
			//lod.meshletCount = addMeshlets(attributes_data, u32_indices);
		}

		auto& vertex_buffer = GetGeometryBuffer(EGeometryBufferIndices::eVertexBuffer);
		auto& index_buffer = GetGeometryBuffer(EGeometryBufferIndices::eIndexBuffer);

		lod.firstIndex = index_buffer->Count();
		lod.indexCount = AddIndices(new_index_ptr);
		lod.firstVertex = vertex_buffer->Count();
		lod.vertexCount = AddVertices(attributes_data);

		lod_buffer->EmplaceBack(lod);
	}

	return new_range;
}

u64 CVertexHandler::AddVertices(const utl::vector<FVertexAttributePointer>& attributes_data)
{
	auto& vertex_buffer = GetGeometryBuffer(EGeometryBufferIndices::eVertexBuffer);

	utl::vector<u8> vertices{};
	auto vertex_count = attributes_data.front().count;
	for (u64 v = 0ull; v < vertex_count; ++v)
	{
		utl::vector<uint8_t> temp(vertex_buffer->Stride());

		u64 offset{ 0ull };
		for (const auto& attrib : attributes_data)
		{
			std::memcpy(temp.data() + offset, &attrib.data[v * attrib.stride], attrib.stride);
			offset += attrib.stride;
		}

		vertices.insert(vertices.end(), temp.begin(), temp.end());
	}

	vertex_buffer->CopyBytes(&vertices[0], vertices.size());

	return vertex_count;
}

u64 CVertexHandler::AddIndices(const FVertexAttributePointer& indices)
{
	auto& index_buffer = GetGeometryBuffer(EGeometryBufferIndices::eIndexBuffer);

	index_buffer->CopyBytes(indices.data, indices.count * indices.stride);
	return indices.count;
}

u64 CVertexHandler::addMeshlets(const utl::vector<FVertexAttributePointer>& attributes, const utl::vector<u32>& indices)
{
	// Find positions attribute
	const float* positions{ nullptr };
	auto found_position_attrib = std::find_if(attributes.begin(), attributes.end(), [](const FVertexAttributePointer& attribute) { return attribute.type == EVertexAttributeType::ePosition; });
	if (found_position_attrib != attributes.end())
		positions = (const float*)found_position_attrib->data;

	// Start generation meshlets
	auto max_meshlets = meshopt_buildMeshletsBound(indices.size(), kMaxVerticesPerMeshlet, kMaxTrianglesPerMeshlet);

	utl::vector<FMeshlet> temp_meshlets(max_meshlets);
	utl::vector<FGeometryDrawBounds> temp_bounds{};
	utl::vector<FTriangle> temp_triangles{};
	utl::vector<u32> temp_vertices(max_meshlets * kMaxVerticesPerMeshlet);

	utl::vector<meshopt_Meshlet> raw_meshlets(max_meshlets);
	utl::vector<u8> raw_meshlet_triangles(max_meshlets * kMaxTrianglesPerMeshlet * 3u);

	size_t meshlet_count = meshopt_buildMeshlets(
		&raw_meshlets[0],
		&temp_vertices[0],
		&raw_meshlet_triangles[0],
		&indices[0],
		indices.size(),
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
		meshlet.triangleOffset = triangle_offset + triangle_buffer->Count();
		meshlet.vertexCount = raw_meshlet.vertex_count;
		meshlet.vertexOffset = raw_meshlet.vertex_offset + vertices_buffer->Count(); // set from first vertex

		triangle_offset += raw_meshlet.triangle_count;
	}
	temp_triangles.resize(triangle_offset);

	auto& meshlet_buffer = m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eMeshletBuffer));
	meshlet_buffer->Copy(temp_meshlets);

	auto& bounds_buffer = m_vGeometryBuffers.at(utl::enum_cast(EGeometryBufferIndices::eBoundsBuffer));
	bounds_buffer->Copy(temp_bounds);

	vertices_buffer->Copy(temp_vertices);
	triangle_buffer->Copy(temp_triangles);

	return meshlet_count;
}

const FVertexDeclarationInfo& CVertexHandler::GetVertexDeclaration() const
{
	return m_declaration;
}

u64 CVertexHandler::GetHash() const
{
	return m_attributeHash;
}

const std::string& CVertexHandler::GetDefinitions() const
{
	return m_definitions;
}

const std::unique_ptr<IGPUVector>& CVertexHandler::GetGeometryBuffer(EGeometryBufferIndices buffer_index)
{
	return m_vGeometryBuffers.at(utl::enum_cast(buffer_index));
}