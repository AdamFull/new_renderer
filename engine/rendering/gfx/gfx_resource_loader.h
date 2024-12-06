#pragma once

#include "gfx_common.h"
#include "gfx_structures.h"

namespace engine
{
	namespace gfx
	{
		class Context;

		struct ResourceLoaderCreateInfo
		{
			Context* context{ nullptr };
			FilesystemWrapper* filesystem_wrapper{ nullptr };

			const char* shader_compile_config_path{ nullptr };
			const char* shader_cache_patch{ nullptr };
		};

		class ResourceLoader
		{
		public:
			virtual ~ResourceLoader() = default;

			virtual Texture* LoadTexture(const char* texture_path) = 0;
			virtual Texture* LoadTexture(const TextureCreateInfo& create_info, std::span<uint8_t> texture_data) = 0;

			virtual Shader* LoadShader(const char* shader_name) = 0;

			virtual void BeginBufferUpdate(BufferUpdateInfo& update_info) = 0;
			virtual void EndBufferUpdate(BufferUpdateInfo& update_info) = 0;

			virtual SemaphoreSubmitInfo Submit(SemaphoreSubmitInfo* wait_infos, uint32_t wait_semaphore_count) = 0;

			virtual Context& GetContext() = 0;
			virtual Context const& GetContext() const = 0;
		};

		std::unique_ptr<ResourceLoader> CreateResourceLoader(const ResourceLoaderCreateInfo& create_info);
	}
}