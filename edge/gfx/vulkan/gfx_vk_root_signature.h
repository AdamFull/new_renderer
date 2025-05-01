#pragma once

#include "../gfx_base.h"
#include "gfx_vk_descriptor_set_layout.h"

namespace edge::gfx::vulkan
{
	class Device;

	struct PipelineLayoutCreateInfo {
	public:
		vk::PipelineLayoutCreateInfo create_info{};

		std::array<vk::DescriptorSetLayout, 32ull> descriptor_set_layouts{};
		std::array<vk::PushConstantRange, 16ull> push_constant_ranges{};

		PipelineLayoutCreateInfo();

		void add_descriptor_set_layout(uint32_t set, vk::DescriptorSetLayout layout);
		vk::PushConstantRange& add_constant_range();
	};

	class PipelineLayout final : public Resource<vk::PipelineLayout> {
	public:
		PipelineLayout(Device& device, const PipelineLayoutCreateInfo& create_info);
		~PipelineLayout();

		PipelineLayout(PipelineLayout&) = delete;
		PipelineLayout(PipelineLayout&& other);

		PipelineLayout& operator=(const PipelineLayout&) = delete;
		PipelineLayout& operator=(PipelineLayout&&) = delete;
	private:
		Device& device;
	};


	class RootSignature final : public IRootSignature {
	public:
		friend class DescriptorAllocator;
		friend class CommandList;

		RootSignature(Device& device, const RootSignatureCreateInfo& create_info);
		~RootSignature();

		RootSignature(RootSignature&) = delete;
		RootSignature(RootSignature&& other);

		RootSignature& operator=(const RootSignature&) = delete;
		RootSignature& operator=(RootSignature&&) = delete;

		void set_resource_name(std::string_view name) const noexcept override;

		[[nodiscard]] auto get_pipeline_layout() noexcept -> PipelineLayout& {
			return *pipeline_layout;
		}

		[[nodiscard]] auto get_descriptor_set_layouts() const noexcept ->
			std::array<std::unique_ptr<DescriptorSetLayout>, k_max_descriptor_count> const& {
			return descriptor_set_layouts;
		}
	private:
		Device& device;

		std::unique_ptr<PipelineLayout> pipeline_layout{};
		std::array<std::unique_ptr<DescriptorSetLayout>, k_max_descriptor_count> descriptor_set_layouts{};
	};
}