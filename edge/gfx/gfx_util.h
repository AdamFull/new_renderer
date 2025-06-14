#pragma once

#include "gfx_structures.h"
#include "../util/utl_hash.hpp"
#include <cassert>

namespace edge::gfx {
	inline ResourceStateFlags get_state_by_descriptor_type(DescriptorType type) {
		switch (type)
		{
		case DescriptorType::eSampler:
		case DescriptorType::eCombinedImageSampler:
		case DescriptorType::eSampledImage:
			return ResourceStateFlag::eShaderResource;
			break;
		case DescriptorType::eStorageImage:
		case DescriptorType::eStorageTexelBuffer:
		case DescriptorType::eStorageBuffer:
		case DescriptorType::eStorageBufferDynamic:
			return ResourceStateFlag::eUnorderedAccess;
			break;
		case DescriptorType::eInputAttachment:
			break;
		default:
			break;
		}

		return ResourceStateFlag::eUndefined;
	}

	inline constexpr uint32_t hash(ShaderStageFlag flag, uint32_t hash = 0u) {
		std::string_view enum_string = enum_to_string(flag);
		return murmur_hash(enum_string, hash);
	}
}