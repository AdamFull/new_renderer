#pragma once

#include "gfx_vk_resource.h"
#include "../gfx_base.h"

namespace edge::gfx::vulkan
{
	class Device;
	class DebugUtils;

	class Pipeline : public IPipeline, public Resource<vk::Pipeline> {
	public:
		Pipeline(Device& device);
		Pipeline(Pipeline&& other);
		virtual ~Pipeline();

		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;
		Pipeline& operator=(Pipeline&&) = delete;

		void set_resource_name(std::string_view name) const noexcept override final;
	protected:
		Device& device;
	};

	class ComputePipeline final : public Pipeline {
	public:
		ComputePipeline(Device& device, const ComputePipelineCreateInfo& create_info);
		ComputePipeline(ComputePipeline&& other);
		~ComputePipeline() = default;

		[[nodescard]] auto get_type() const noexcept -> PipelineType override {
			return PipelineType::eCompute;
		}
	};

	class GraphicsPipeline final : public Pipeline {
	public:
		GraphicsPipeline(Device& device, const GraphicsPipelineCreateInfo& create_info);
		GraphicsPipeline(GraphicsPipeline&& other);
		~GraphicsPipeline() = default;

		[[nodescard]] auto get_type() const noexcept -> PipelineType override {
			return PipelineType::eGraphics;
		}
	};
}