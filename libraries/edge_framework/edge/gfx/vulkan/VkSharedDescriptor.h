#pragma once

#include "VkDescriptorSet.h"
#include "../gfx_base.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class DescriptorPool;
			class VulkanResourceDescriptor;

			class VulkanSharedDescriptor final : public ISharedDescriptor {
			public:
				VulkanSharedDescriptor(Device& device, const SharedDescriptorCreateInfo& create_info);
				~VulkanSharedDescriptor();

				VulkanSharedDescriptor(const VulkanSharedDescriptor&) = delete;

				VulkanSharedDescriptor(VulkanSharedDescriptor&& other);

				VulkanSharedDescriptor& operator=(const VulkanSharedDescriptor&) = delete;
				VulkanSharedDescriptor& operator=(VulkanSharedDescriptor&&) = delete;

				[[nodiscard]] auto get_descriptor_set_layout() const noexcept -> DescriptorSetLayout& {
					return *descriptor_set_layout;
				}

				//DescriptorSet& GetDescriptorSet();

				[[nodiscard]] uint32_t get_first_set() const noexcept {
					return first_set;
				}

				void set_resource_name(std::string_view name) const noexcept override;

				void reset() override;
				[[nodiscard]] auto get_latest_descriptors() const noexcept -> IResourceDescriptor* override;

				[[nodiscard]] auto request_descriptors() -> IResourceDescriptor* override;
			private:
				Device& device;

				uint32_t first_set{};

				std::unique_ptr<DescriptorSetLayout> descriptor_set_layout{};
				std::unique_ptr<DescriptorPool> descriptor_pool{};

				std::vector<VulkanResourceDescriptor*> allocated_resource_descriptors{};
			};
		}
	}
}