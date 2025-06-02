#include "gfx_vk_acceleration_structure.h"
#include "gfx_vk_buffer.h"
#include "gfx_vk_device.h"
#include "gfx_vk_debug.h"
#include "gfx_vk_util.h"

namespace edge::gfx::vulkan {
	AccelerationStructure::AccelerationStructure(Device& device, const AccelerationStructureCreateInfo& create_info) :
		Resource{ nullptr, device.get_allocation_callbacks() },
		device{ device } {

		vk::AccelerationStructureCreateInfoKHR as_create_info{};
		as_create_info.buffer = create_info.buffer->as<Buffer*>()->get_handle();
		as_create_info.offset = create_info.offset;
		as_create_info.size = create_info.size;
		as_create_info.deviceAddress = create_info.buffer->get_address();
		as_create_info.type = acceleration_structure_type_to_vk(create_info.type);

		vk::Result result = device.get_handle().createAccelerationStructureKHR(&as_create_info, allocation_callbacks, &handle_);
	}

	AccelerationStructure::~AccelerationStructure() {
		device.get_handle().destroyAccelerationStructureKHR(handle_, allocation_callbacks);
	}

	void AccelerationStructure::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();
		debug_utils.set_debug_name(handle_, name);
	}

	auto AccelerationStructure::add_vertex_geometry(IBuffer* vertex_buffer, IBuffer* index_buffer, IBuffer* transform_buffer,
		uint32_t triangle_count, uint32_t max_vertex, uint64_t vertex_stride, uint32_t transform_offset,
		Format vertex_format, vk::GeometryFlagsKHR flags) -> uint64_t {

		vk::AccelerationStructureGeometryKHR geometry{};

		geometry.geometryType = vk::GeometryTypeKHR::eTriangles;
		geometry.flags = flags;
		geometry.geometry.triangles.vertexFormat = static_cast<vk::Format>(format_to_vk(vertex_format));
		geometry.geometry.triangles.maxVertex = max_vertex;
		geometry.geometry.triangles.vertexStride = vertex_stride;
		// TODO: Vertex format need to be set
		geometry.geometry.triangles.indexType = vk::IndexType::eUint16;
		geometry.geometry.triangles.vertexData.deviceAddress = vertex_buffer->get_address();
		geometry.geometry.triangles.indexData.deviceAddress = index_buffer->get_address();
		geometry.geometry.triangles.transformData.deviceAddress = transform_buffer->get_address();

		uint64_t index = geometries.size();
		geometries.push_back({ geometry, triangle_count, transform_offset });
		return index;
	}

	void AccelerationStructure::update_vertex_geometry(uint64_t triangleUUID, IBuffer* vertex_buffer, IBuffer* index_buffer, IBuffer* transform_buffer,
		uint32_t triangle_count, uint32_t max_vertex, uint64_t vertex_stride, uint32_t transform_offset,
		Format vertex_format, vk::GeometryFlagsKHR flags) {

		auto& geometry = geometries[triangleUUID];
		geometry.geometry.geometryType = vk::GeometryTypeKHR::eTriangles;
		geometry.geometry.flags = flags;
		geometry.geometry.geometry.triangles.vertexFormat = static_cast<vk::Format>(format_to_vk(vertex_format));
		geometry.geometry.geometry.triangles.maxVertex = max_vertex;
		geometry.geometry.geometry.triangles.vertexStride = vertex_stride;
		// TODO: Vertex format need to be set
		geometry.geometry.geometry.triangles.indexType = vk::IndexType::eUint16;
		geometry.geometry.geometry.triangles.vertexData.deviceAddress = vertex_buffer->get_address();
		geometry.geometry.geometry.triangles.indexData.deviceAddress = index_buffer->get_address();
		geometry.geometry.geometry.triangles.transformData.deviceAddress = transform_buffer->get_address();
		geometry.primitive_count = triangle_count;
		geometry.transform_offset = transform_offset;
		geometry.updated = true;
	}

	auto AccelerationStructure::add_instance_geometry(IBuffer* instance_buffer, uint32_t instance_count, uint32_t transform_offset, vk::GeometryFlagsKHR flags) -> uint64_t {
		vk::AccelerationStructureGeometryKHR geometry{};
		geometry.geometryType = vk::GeometryTypeKHR::eInstances;
		geometry.flags = flags;
		geometry.geometry.instances.arrayOfPointers = VK_FALSE;
		geometry.geometry.instances.data.deviceAddress = instance_buffer->get_address();

		uint64_t index = geometries.size();
		geometries.push_back({ geometry, instance_count, transform_offset });
		return index;
	}

	void AccelerationStructure::update_instance_geometry(uint64_t instance_UID, IBuffer* instance_buffer, uint32_t instance_count,
		uint32_t transform_offset, vk::GeometryFlagsKHR flags) {

		auto& geometry = geometries[instance_UID];
		geometry.geometry.geometryType = vk::GeometryTypeKHR::eInstances;
		geometry.geometry.flags = flags;
		geometry.geometry.geometry.instances.arrayOfPointers = VK_FALSE;
		geometry.geometry.geometry.instances.data.deviceAddress = instance_buffer->get_address();
		geometry.primitive_count = instance_count;
		geometry.transform_offset = transform_offset;
		geometry.updated = true;
	}

	void AccelerationStructure::build(const IQueue* queue, vk::BuildAccelerationStructureFlagsKHR flags, vk::BuildAccelerationStructureModeKHR mode) {
		std::vector<vk::AccelerationStructureGeometryKHR> acceleration_structure_geometries{};
		std::vector<vk::AccelerationStructureBuildRangeInfoKHR> acceleration_structure_build_range_infos{};
		std::vector<uint32_t> primitive_counts{};

		for (auto& geometry : geometries) {
			if (mode == vk::BuildAccelerationStructureModeKHR::eUpdate && !geometry.updated) {
				continue;
			}
			acceleration_structure_geometries.push_back(geometry.geometry);
			// Infer build range info from geometry
			vk::AccelerationStructureBuildRangeInfoKHR build_range_info;
			build_range_info.primitiveCount = geometry.primitive_count;
			build_range_info.primitiveOffset = 0;
			build_range_info.firstVertex = 0;
			build_range_info.transformOffset = geometry.transform_offset;
			acceleration_structure_build_range_infos.push_back(build_range_info);
			primitive_counts.push_back(geometry.primitive_count);
			geometry.updated = false;
		}
	}

	auto AccelerationStructure::get_address() const -> uint64_t {
		vk::AccelerationStructureDeviceAddressInfoKHR get_address_info{};
		get_address_info.accelerationStructure = handle_;

		return device.get_handle().getAccelerationStructureAddressKHR(&get_address_info);
	}
}