#pragma once

#include "gfx_base.h"

namespace edge::gfx {
	class DescriptorTable {
	public:
		DescriptorTable() = default;
		DescriptorTable(IDescriptorAllocator* allocator);

		void add_layout(uint32_t set, IDescriptorLayout* layout);

		auto bind(uint32_t set, const SamplerDescriptor& sampler_desc) -> void;
		auto bind(uint32_t set, const TextureDescriptor& texture_desc) -> void;
		auto bind(uint32_t set, const BufferDescriptor& buffer_desc) -> void;

		auto invalidate() -> void;
		auto reset() -> void;

		auto get_layouts() noexcept -> std::span<IDescriptorLayout*>;
	private:
		auto get_layout(uint32_t set) const noexcept -> IDescriptorLayout*;

		IDescriptorAllocator* allocator{ nullptr };
		std::vector<IDescriptorLayout*> layouts;
	};
}