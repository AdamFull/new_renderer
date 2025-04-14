#include "gfx_descriptor_table.h"

#include "../util/Logger.h"

namespace edge::gfx {
	DescriptorTable::DescriptorTable(IDescriptorAllocator* allocator) :
		allocator{ allocator } {

	}

	void DescriptorTable::add_layout(uint32_t set, IDescriptorLayout* layout) {
		if (set >= layouts.size()) {
			layouts.resize(set + 1, nullptr);
		}
		layouts[set] = layout;
	}

	void DescriptorTable::bind(uint32_t set, const SamplerDescriptor& sampler_desc) {
		invalidate();
		auto* layout = get_layout(set);
		layout->bind(sampler_desc);
	}

	void DescriptorTable::bind(uint32_t set, const TextureDescriptor& texture_desc) {
		invalidate();
		auto* layout = get_layout(set);
		layout->bind(texture_desc);
	}

	void DescriptorTable::bind(uint32_t set, const BufferDescriptor& buffer_desc) {
		invalidate();
		auto* layout = get_layout(set);
		layout->bind(buffer_desc);
	}

	auto DescriptorTable::invalidate() -> void {
		// Check layouts that is invalidated
		std::vector<IDescriptorLayout*> invalidated_layouts{};
		for (auto& layout : layouts) {
			if (!layout->valid() || !layout->allocated()) {
				invalidated_layouts.push_back(layout);
			}
		}

		if (!invalidated_layouts.empty()) {
			allocator->allocate(invalidated_layouts);
		}
	}

	auto DescriptorTable::reset() -> void {
		std::vector<IDescriptorLayout*> layouts_to_free{};

		// Deallocate all invalidated layouts
		for (auto& layout : layouts) {
			if (layout->allocated() && !layout->valid()) {
				layouts_to_free.push_back(layout);
			}
		}
		
		if (!layouts_to_free.empty()) {
			allocator->free(layouts_to_free);
		}		
	}

	auto DescriptorTable::get_layouts() noexcept -> std::span<IDescriptorLayout*> {
		return layouts;
	}

	auto DescriptorTable::get_layout(uint32_t set) const noexcept -> IDescriptorLayout* {
		assert(set < layouts.size() && "Layout not found.");

		if (set < layouts.size())
			return layouts[set];
		return nullptr;
	}
}