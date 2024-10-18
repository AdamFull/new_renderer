#pragma once

#include <engine/render/render_api/vulkan/VkResource.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			static constexpr uint32_t k_max_pool_sizes_count{ 11u };
			static constexpr uint32_t k_max_layout_bindings{ 16u };

			using PoolSizes = std::array<int32_t, k_max_pool_sizes_count>;

			class RenderDevice;
			class DescriptorSetLayout;

			class DescriptorSetLayoutBuilder
			{
			public:
				DescriptorSetLayoutBuilder();

				std::string debug_name{};

				vk::DescriptorSetLayoutCreateInfo create_info{};
				vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT ext_create_info{};

				std::array<vk::DescriptorSetLayoutBinding, k_max_layout_bindings> bindings{};
				std::array<vk::DescriptorBindingFlagsEXT, k_max_layout_bindings> binding_flags_ext{};

				DescriptorSetLayoutBuilder& SetDebugName(const std::string& name);
				DescriptorSetLayoutBuilder& AddLayoutBinding(const vk::DescriptorSetLayoutBinding& binding, vk::DescriptorBindingFlagsEXT flags = {});
				DescriptorSetLayoutBuilder& SetCreateFlags(vk::DescriptorSetLayoutCreateFlags flags);

				DescriptorSetLayout Build(RenderDevice& render_device);
				std::unique_ptr<DescriptorSetLayout> BuildUnique(RenderDevice& render_device);
			};

			class DescriptorSetLayout final : public VulkanResource<vk::DescriptorSetLayout>
			{
			public:
				struct DescriptorBindingInfo
				{
					vk::DescriptorSetLayoutBinding binding{};
					vk::DescriptorBindingFlagsEXT flags{};
				};

				using LayoutBindingArray = std::array<DescriptorBindingInfo, k_max_layout_bindings>;

				DescriptorSetLayout(RenderDevice& render_device, const DescriptorSetLayoutBuilder& builder);
				DescriptorSetLayout(const DescriptorSetLayout&) = delete;
				DescriptorSetLayout(DescriptorSetLayout&& other);
				~DescriptorSetLayout();

				DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
				DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;

				const LayoutBindingArray& GetBindings() const;
				uint32_t GetBindingCount() const;

				vk::DescriptorSetLayoutBinding& GetLayoutBinding(uint32_t binding_index);
				vk::DescriptorBindingFlagsEXT& GetLayoutBindingFlags(uint32_t binding_index);

				const PoolSizes& GetRequiredDescriptorCount() const;
			private:
				RenderDevice& render_device;

				PoolSizes pool_sizes{};
				LayoutBindingArray layout_bindings{};
				uint32_t binding_count{ 0u };
			};
		}
	}
}