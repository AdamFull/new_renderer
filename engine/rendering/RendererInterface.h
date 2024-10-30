#pragma once

#include <engine/rendering/GeometryBufferInterface.h>

namespace engine
{
	namespace render
	{
		class IRenderer
		{
		public:
			virtual ~IRenderer() = default;

			virtual void Render(float delta_time) = 0;

			virtual uint32_t AddRenderResource(const std::string& path, gfx::Sampler* sampler = nullptr) = 0;
			virtual uint32_t AddRenderResource(const gfx::TextureCreateInfo& create_info, gfx::Sampler* sampler = nullptr) = 0;

			virtual uint32_t AddRenderResource(const gfx::BufferCreateInfo& create_info) = 0;

			virtual gfx::Sampler* GetOrCreateSampler(const gfx::SamplerCreateInfo& create_info) = 0;

			virtual GeometryBuffer& RequestInterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) = 0;
			virtual GeometryBuffer& RequestDeinterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) = 0;
		};
	}
}