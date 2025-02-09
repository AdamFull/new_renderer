#pragma once

#include "../../gfx_common.h"
#include "../../gfx_structures.h"

namespace edge
{
	namespace gfx
	{
		class Context;

		namespace ext
		{
			struct ResourceLoaderCreateInfo
			{
				Context* context{ nullptr };
				FilesystemWrapper* filesystem_wrapper{ nullptr };
			};

			//class BufferUpdater
			//{
			//public:
			//	void Update(const void* data, uint64_t data_size, uint64_t dst_offset = 0ull)
			//	{
			//		copy_regions.emplace_back(src_offset, dst_offset, data_size);
			//		src_buffer->Update(data, data_size, src_offset);
			//		src_offset += data_size;
			//	}
			//private:
			//	Buffer* src_buffer{ nullptr };
			//	uint64_t src_offset{ 0ull };
			//
			//	Buffer* dst_buffer{ nullptr };
			//
			//	std::vector<BufferCopyRegion> copy_regions{};
			//};

			class ResourceLoader
			{
			public:
				virtual ~ResourceLoader() = default;

				virtual Texture* LoadTexture(std::string_view texture_path) = 0;
				virtual Texture* LoadTexture(const TextureCreateInfo& create_info, std::span<uint8_t> texture_data) = 0;

				virtual Sampler* GetSampler(const SamplerCreateInfo& create_info) = 0;

				virtual void BeginBufferUpdate(BufferUpdateInfo& update_info) = 0;
				virtual void EndBufferUpdate(BufferUpdateInfo& update_info) = 0;

				virtual SemaphoreSubmitInfo Submit(std::span<SemaphoreSubmitInfo> wait_infos) = 0;

				virtual Context& GetContext() = 0;
				virtual Context const& GetContext() const = 0;
			};

			std::unique_ptr<ResourceLoader> CreateResourceLoader(const ResourceLoaderCreateInfo& create_info);
		}
	}
}