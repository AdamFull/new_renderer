#pragma once

#include "VkResource.h"
#include "../gfx_base.h"

namespace edge::gfx::vulkan
{
	class Device;
	class DebugUtils;

	class VulkanPipeline : public IPipeline, public VulkanResource<vk::Pipeline> {
	public:
		VulkanPipeline(Device& device);
		VulkanPipeline(VulkanPipeline&& other);
		virtual ~VulkanPipeline();

		VulkanPipeline(const VulkanPipeline&) = delete;
		VulkanPipeline& operator=(const VulkanPipeline&) = delete;
		VulkanPipeline& operator=(VulkanPipeline&&) = delete;

		void set_resource_name(std::string_view name) const noexcept override final;
	protected:
		Device& device;
	};

	class VulkanComputePipeline final : public VulkanPipeline {
	public:
		VulkanComputePipeline(Device& device, const ComputePipelineCreateInfo& create_info);
		VulkanComputePipeline(VulkanComputePipeline&& other);
		~VulkanComputePipeline() = default;

		[[nodescard]] auto get_type() const noexcept -> PipelineType override {
			return PipelineType::eCompute;
		}
	};

	class VulkanGraphicsPipeline final : public VulkanPipeline {
	public:
		VulkanGraphicsPipeline(Device& device, const GraphicsPipelineCreateInfo& create_info);
		VulkanGraphicsPipeline(VulkanGraphicsPipeline&& other);
		~VulkanGraphicsPipeline() = default;

		[[nodescard]] auto get_type() const noexcept -> PipelineType override {
			return PipelineType::eGraphics;
		}
	};
}