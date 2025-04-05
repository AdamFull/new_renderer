#pragma once

#include "../gfx_base.h"

namespace edge::gfx::vulkan
{
	class Device;
	class DescriptorPool;
	class DescriptorSetLayout;
	class DescriptorSet;
	class ResourceDescriptor;

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
		auto allocate_descriptors(IRootSignature* root_signature) -> IResourceDescriptor* override;
	private:
		auto allocate(DescriptorSetLayout& layout) -> DescriptorSet&;
	private:
		Device& device;

		// TODO: Do i even need to allocate memory here?
		uint32_t multiplicator{ 2u };
		std::vector<std::unique_ptr<DescriptorPool>> descriptor_pools{};
		std::vector<ResourceDescriptor*> allocated_descriptors{};
	};
}