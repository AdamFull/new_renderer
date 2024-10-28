#pragma once

#include <engine/rendering/gfx/Structures.h>

namespace engine
{
	namespace render
	{
		class IRenderer
		{
		public:
			virtual ~IRenderer() = default;

			virtual void Render(float delta_time) = 0;

			virtual uint32_t AddTextureResource(const std::string& path, gfx::Sampler* sampler = nullptr) = 0;
			virtual uint32_t AddTextureResource(const gfx::TextureCreateInfo& create_info, gfx::Sampler* sampler = nullptr) = 0;

			virtual uint32_t AddBufferResource(const gfx::BufferCreateInfo& create_info) = 0;

			virtual gfx::Sampler* GetOrCreateSampler(const gfx::SamplerCreateInfo& create_info) = 0;
		};
	}
}