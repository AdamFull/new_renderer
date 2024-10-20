#include <base/PipelineLayout.h>
#include <VkRenderDevice.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			PipelineLayoutBuilder::PipelineLayoutBuilder()
			{
				create_info.pSetLayouts = descriptor_set_layouts.data();
				create_info.pPushConstantRanges = push_constant_ranges.data();
			}

			PipelineLayoutBuilder& PipelineLayoutBuilder::SetDebugName(const std::string& name)
			{
				debug_name = name;
				return *this;
			}

			PipelineLayoutBuilder& PipelineLayoutBuilder::SetFlags(vk::PipelineLayoutCreateFlags flags)
			{
				create_info.flags = flags;
				return *this;
			}

			PipelineLayoutBuilder& PipelineLayoutBuilder::AddDescriptorSetLayout(vk::DescriptorSetLayout layout)
			{
				descriptor_set_layouts[create_info.setLayoutCount++] = layout;
				return *this;
			}

			PipelineLayoutBuilder& PipelineLayoutBuilder::AddPushConstantRange(const vk::PushConstantRange& range)
			{
				push_constant_ranges[create_info.pushConstantRangeCount++] = range;
				return *this;
			}

			PipelineLayout PipelineLayoutBuilder::Build(RenderDevice& render_device) const
			{
				return PipelineLayout(render_device, *this);
			}

			std::unique_ptr<PipelineLayout> PipelineLayoutBuilder::BuildUnique(RenderDevice& render_device) const
			{
				return std::make_unique<PipelineLayout>(render_device, *this);
			}


			PipelineLayout::PipelineLayout(RenderDevice& render_device, const PipelineLayoutBuilder& builder) :
				VulkanResource{ nullptr },
				render_device{ render_device }
			{
				handle = render_device.CreatePipelineLayout2(builder.create_info, builder.debug_name);
			}

			PipelineLayout::PipelineLayout(PipelineLayout&& other) :
				VulkanResource{ std::move(other) },
				render_device{ other.render_device }
			{
			}

			PipelineLayout::~PipelineLayout()
			{
				render_device.Destroy(handle);
			}
		}
	}
}