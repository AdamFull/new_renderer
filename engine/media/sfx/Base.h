#pragma once

#include "Structures.h"

namespace engine
{
	namespace sfx
	{
		class SFXResource
		{
		public:
			virtual ~SFXResource() = default;

			virtual ResourceType get_resource_type() const = 0;
		};

		class Buffer : public SFXResource
		{
		public:
			~Buffer() = default;

			ResourceType get_resource_type() const
			{
				return ResourceType::eBuffer;
			}

			virtual uint64_t Update(const void* data, uint64_t size) = 0;
		};

		class Source : public SFXResource
		{
		public:
			~Source() = default;

			ResourceType get_resource_type() const
			{
				return ResourceType::eSource;
			}
		};

		class Listener : public SFXResource
		{
		public:
			~Listener() = default;

			ResourceType get_resource_type() const
			{
				return ResourceType::eListener;
			}
		};
	}
}