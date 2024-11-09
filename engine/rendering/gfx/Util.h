#pragma once

#include <Structures.h>
#include <cassert>

namespace engine
{
	namespace gfx
	{
		inline ResourceStateFlags GetAccessState(AccessTypeFlags flags)
		{
			ResourceStateFlags state_flags{};
			if (flags & AccessTypeFlag::eRead)
				state_flags |= ResourceStateFlag::eRead;
			if (flags & AccessTypeFlag::eWrite)
				state_flags |= ResourceStateFlag::eWrite;

			return state_flags;
		}

		inline ResourceStateFlag GetStateByDescriptorType(DescriptorType type)
		{
			switch (type)
			{
			case engine::gfx::DescriptorType::eSampler:
			case engine::gfx::DescriptorType::eCombinedImageSampler:
			case engine::gfx::DescriptorType::eSampledImage:
				return ResourceStateFlag::eSample;
				break;
			case engine::gfx::DescriptorType::eStorageImage:
			case engine::gfx::DescriptorType::eStorageTexelBuffer:
			case engine::gfx::DescriptorType::eStorageBuffer:
				return ResourceStateFlag::eStorage;
				break;
			case engine::gfx::DescriptorType::eStorageBufferDynamic:
				return ResourceStateFlag::eStorage;
				break;
			case engine::gfx::DescriptorType::eInputAttachment:
				break;
			default:
				break;
			}

			return ResourceStateFlag::eUndefined;
		}
	}
}