#pragma once

#include "gfx_structures.h"
#include "../util/Hash.hpp"
#include <cassert>

namespace edge
{
	namespace gfx
	{
		inline ResourceStateFlags get_access_state(AccessTypeFlags flags) {
			ResourceStateFlags state_flags{};
			if (flags & AccessTypeFlag::eRead)
				state_flags |= ResourceStateFlag::eRead;
			if (flags & AccessTypeFlag::eWrite)
				state_flags |= ResourceStateFlag::eWrite;

			return state_flags;
		}

		inline ResourceStateFlag get_state_by_descriptor_type(DescriptorType type) {
			switch (type)
			{
			case DescriptorType::eSampler:
			case DescriptorType::eCombinedImageSampler:
			case DescriptorType::eSampledImage:
				return ResourceStateFlag::eSample;
				break;
			case DescriptorType::eStorageImage:
			case DescriptorType::eStorageTexelBuffer:
			case DescriptorType::eStorageBuffer:
				return ResourceStateFlag::eStorage;
				break;
			case DescriptorType::eStorageBufferDynamic:
				return ResourceStateFlag::eStorage;
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
}