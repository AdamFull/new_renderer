#pragma once

#include <string>
#include <engine/rendering/gfx/GFXStructures.h>

namespace engine
{
	namespace gfx
	{
		struct BufferUpdateInfo
		{
			Buffer* dst_buffer{ nullptr };
			uint64_t update_size{ 0ull };

			std::array<BufferCopyRegion, 64ull> regions{};
			uint32_t region_count{};

			struct
			{
				uint8_t* data{ nullptr };
				uint64_t avaliable_size{ 0ull };
				size_t buffer_index{ ~0ull };
			} map_info;
		};

		class ResourceContext
		{
		public:
			~ResourceContext() = default;

			virtual Texture* LoadTexture(const std::string& texture_path) = 0;
			virtual Texture* CreateTexture(const TextureCreateInfo& create_info) = 0;

			virtual Sampler* CreateSampler(const SamplerCreateInfo& create_info) = 0;

			virtual Buffer* CreateBuffer(const BufferCreateInfo& create_info) = 0;

			virtual void DestroyResource(GFXResource* resource) = 0;

			virtual void BeginBufferUpdate(BufferUpdateInfo& update_info) = 0;
			virtual void EndBufferUpdate(BufferUpdateInfo& update_info) = 0;

			virtual SemaphoreSubmitInfo Submit(SemaphoreSubmitInfo* wait_infos, uint32_t wait_semaphore_count) = 0;
		};
	}
}