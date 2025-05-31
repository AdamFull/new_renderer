#pragma once

#include "gfx_vk_resource.h"
#include "../gfx_base.h"

namespace edge::gfx::vulkan {
	class Device;
	class Buffer;

	class AccelerationStructure final : public IAccelerationStructure, public Resource<vk::AccelerationStructureKHR> {
	public:
		AccelerationStructure(Device& device, const AccelerationStructureCreateInfo& create_info);
		~AccelerationStructure();

		void set_resource_name(std::string_view name) const noexcept override;

		auto add_vertex_geometry(IBuffer* vertex_buffer, IBuffer* index_buffer, IBuffer* transform_buffer,
			uint32_t triangle_count, uint32_t max_vertex, uint64_t vertex_stride, uint32_t transform_offset,
			Format vertex_format, vk::GeometryFlagsKHR flags) -> uint64_t;

		void update_vertex_geometry(uint64_t triangleUUID, IBuffer* vertex_buffer, IBuffer* index_buffer, IBuffer* transform_buffer,
			uint32_t triangle_count, uint32_t max_vertex, uint64_t vertex_stride, uint32_t transform_offset,
			Format vertex_format, vk::GeometryFlagsKHR flags);

		auto add_instance_geometry(IBuffer* instance_buffer, uint32_t instance_count, uint32_t transform_offset, vk::GeometryFlagsKHR flags) -> uint64_t;
		void update_instance_geometry(uint64_t instance_UID, IBuffer* instance_buffer, uint32_t instance_count,
			uint32_t transform_offset, vk::GeometryFlagsKHR flags);

		void build(const IQueue* queue, vk::BuildAccelerationStructureFlagsKHR flags, vk::BuildAccelerationStructureModeKHR mode);

		auto get_address() const -> uint64_t override;

		AccelerationStructure(const AccelerationStructure&) = delete;
		AccelerationStructure(AccelerationStructure&& other) = default;
		AccelerationStructure& operator=(const AccelerationStructure&) = delete;
		AccelerationStructure& operator=(AccelerationStructure&&) = default;
	private:
		Device& device;

		vk::AccelerationStructureBuildSizesInfoKHR build_sizes_info{};

		struct ASGeometry {
			vk::AccelerationStructureGeometryKHR geometry{};
			uint32_t primitive_count{};
			uint32_t transform_offset{};
			bool updated{ false };
		};

		std::unique_ptr<Buffer> scratch_buffer{};
		std::vector<ASGeometry> geometries{};
	};
}