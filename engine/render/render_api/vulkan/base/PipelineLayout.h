#pragma once

#include <VkResource.h>
#include <ResourceStructures.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class RenderDevice;
			class PipelineLayout;

			class PipelineLayoutBuilder final
			{
			public:
				std::string debug_name{};
				vk::PipelineLayoutCreateInfo create_info{};

				std::array<vk::DescriptorSetLayout, k_max_descriptor_sets> descriptor_set_layouts{};
				std::array<vk::PushConstantRange, k_max_push_constant_range_count> push_constant_ranges{};

				PipelineLayoutBuilder();

				PipelineLayoutBuilder& SetDebugName(const std::string& name);
				PipelineLayoutBuilder& SetFlags(vk::PipelineLayoutCreateFlags flags);
				PipelineLayoutBuilder& AddDescriptorSetLayout(vk::DescriptorSetLayout layout);
				PipelineLayoutBuilder& AddPushConstantRange(const vk::PushConstantRange& range);

				PipelineLayout Build(RenderDevice& render_device) const;
				std::unique_ptr<PipelineLayout> BuildUnique(RenderDevice& render_device) const;
			};

			class PipelineLayout final : public VulkanResource<vk::PipelineLayout>
			{
			public:
				PipelineLayout(RenderDevice& render_device, const PipelineLayoutBuilder& builder);
				PipelineLayout(PipelineLayout&& other);
				~PipelineLayout();

				PipelineLayout(const PipelineLayout&) = delete;
				PipelineLayout& operator=(const PipelineLayout&) = delete;
				PipelineLayout& operator=(PipelineLayout&&) = delete;
			private:
				RenderDevice& render_device;
			};
		}
	}
}