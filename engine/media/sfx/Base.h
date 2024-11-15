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

			virtual ResourceType GetResourceType() const = 0;
		};

		class Buffer : public SFXResource
		{
		public:
			~Buffer() = default;

			ResourceType GetResourceType() const
			{
				return ResourceType::eBuffer;
			}

			virtual uint64_t Update(const void* data, uint64_t size) = 0;
		};

		class Source : public SFXResource
		{
		public:
			~Source() = default;

			ResourceType GetResourceType() const
			{
				return ResourceType::eSource;
			}
		};

		class Listener : public SFXResource
		{
		public:
			~Listener() = default;

			ResourceType GetResourceType() const
			{
				return ResourceType::eListener;
			}
		};
	}
}