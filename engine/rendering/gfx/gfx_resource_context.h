#pragma once

#include <string>
#include "gfx_structures.h"

namespace engine
{
	namespace gfx
	{
		class Context;

		class ResourceContext
		{
		public:
			virtual ~ResourceContext() = default;

			virtual Texture* LoadTexture(const std::string& texture_path) = 0;
			virtual Texture* LoadTexture(const TextureCreateInfo& create_info, std::span<uint8_t> texture_data) = 0;

			virtual Shader* LoadShader(const std::string& shader_name) = 0;

			virtual Sampler* GetSampler(const SamplerCreateInfo& create_info) = 0;

			virtual void BeginBufferUpdate(BufferUpdateInfo& update_info) = 0;
			virtual void EndBufferUpdate(BufferUpdateInfo& update_info) = 0;

			virtual SemaphoreSubmitInfo Submit(SemaphoreSubmitInfo* wait_infos, uint32_t wait_semaphore_count) = 0;

			virtual Context& GetContext() = 0;
			virtual Context const& GetContext() const = 0;
		};
	}
}