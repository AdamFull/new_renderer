#pragma once

#include <engine/rendering/GeometryBufferInterface.h>
#include <engine/rendering/RenderCameraInterface.h>
#include <engine/rendering/DynamicBufferInterface.h>

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

			virtual gfx::Context& GetContext() = 0;

			virtual GeometryBuffer& RequestInterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) = 0;
			virtual GeometryBuffer& RequestDeinterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) = 0;

			virtual uint32_t AddRenderCamera() = 0;
			virtual IRenderCamera& GetRenderCamera(uint32_t camera_id) = 0;

			virtual IDynamicBuffer& GetNodeBuffer() = 0;
			virtual IDynamicBuffer& GetMatrixBuffer() = 0;
			virtual IDynamicBuffer& GetMeshBuffer() = 0;

			virtual IDynamicBuffer& GetMaterialBuffer() = 0;
			virtual IDynamicBuffer& GetLightBuffer() = 0;

			virtual IDynamicBuffer& GetCascadedShadowBuffer() = 0;
			virtual IDynamicBuffer& GetOmniShadowBuffer() = 0;
			virtual IDynamicBuffer& GetSpotShadowBuffer() = 0;

			virtual IDynamicBuffer& GetGeobetryBatchBuffer() = 0;
			virtual IDynamicBuffer& GetRawGeobetryBatchBuffer() = 0;

			virtual IDynamicBuffer& GetFrustumBuffer() = 0;

			virtual IDynamicBuffer& GetIndirectDrawBuffer() = 0;
			virtual IDynamicBuffer& GetIndirectCountDrawBuffer() = 0;
			virtual IDynamicBuffer& GetMeshCommandBuffer() = 0;

			virtual IDynamicBuffer& GetCameraBuffer() = 0;
		};
	}
}