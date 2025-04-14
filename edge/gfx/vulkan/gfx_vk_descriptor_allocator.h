#pragma once

#include "../gfx_base.h"
#include "gfx_vk_descriptor_set_layout.h"

namespace edge::gfx::vulkan
{
	class Device;

	struct DescriptorPoolBlock {
		vk::DescriptorPool pool{};
		PoolSizes inilial_sizes{};
		PoolSizes avaliable_sizes{};
		int32_t avaliable_sets{ 1 };
		bool bindless_support{ false };
	};

	class DescriptorAllocator final : public IDescriptorAllocator {
	public:
		DescriptorAllocator(Device& device, uint32_t grow_factor = 2u);
		DescriptorAllocator(const DescriptorAllocator&) = delete;
		DescriptorAllocator(DescriptorAllocator&& other);
		~DescriptorAllocator();

		DescriptorAllocator& operator=(const DescriptorAllocator&) = delete;
		DescriptorAllocator& operator=(DescriptorAllocator&&) = delete;

		void set_resource_name(std::string_view name) const noexcept override;

		void reset() override;
		auto allocate(std::span<IDescriptorLayout*> layouts) -> bool override;
		auto free(std::span<IDescriptorLayout*> layouts) -> bool override;
	private:
		auto create_new_pool(const PoolSizes& requested_sizes, bool bindless_support = false) -> DescriptorPoolBlock&;
		auto lookup_free_pool(const PoolSizes& requested_sizes, int32_t requested_set_count, bool bindless_support = false) -> std::tuple<DescriptorPoolBlock&, size_t>;
	private:
		Device& device;

		std::vector<DescriptorPoolBlock> pool_blocks{};
		std::vector<IDescriptorLayout*> allocated_layouts{};
	};
}