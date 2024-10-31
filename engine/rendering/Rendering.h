#pragma once

#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/rendering/RendererInterface.h>
#include <engine/rendering/RenderResourceContainer.h>

#include <engine/rendering/ExecutionContext.h>

//#include <engine/rendering/GPUVector.h>
#include <engine/rendering/render_graph/ShaderPass.h>
#include <engine/rendering/render_graph/RenderGraphInterface.h>
#include <memory>

namespace engine
{
	class Window;

	namespace gfx
	{
		class Context;
		class QueryPool;
		class Sampler;
		class ResourceContext;
	}

	namespace render
	{
		class Renderer : public IRenderer
		{
		public:
			Renderer(Window* window, filesystem::IVirtualFileSystemInterface* vfs);
			~Renderer();

			uint32_t AddRenderResource(const std::string& path, gfx::Sampler* sampler = nullptr) override;
			uint32_t AddRenderResource(const gfx::TextureCreateInfo& create_info, gfx::Sampler* sampler = nullptr) override;

			uint32_t AddRenderResource(const gfx::BufferCreateInfo& create_info) override;

			void Render(float delta_time) override;

			gfx::ResourceContext& GetResourceContext() override;
			gfx::ResourceContext const& GetResourceContext() const override;

			gfx::Context& GetContext() override;

			GeometryBuffer& RequestInterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) override;
			GeometryBuffer& RequestDeinterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) override;

			uint32_t AddRenderCamera() override;
			IRenderCamera& GetRenderCamera(uint32_t camera_id) override;

			IDynamicBuffer& GetNodeBuffer() override;
			IDynamicBuffer& GetMatrixBuffer() override;
			IDynamicBuffer& GetMeshBuffer() override;

			IDynamicBuffer& GetMaterialBuffer() override;
			IDynamicBuffer& GetLightBuffer() override;

			IDynamicBuffer& GetCascadedShadowBuffer() override;
			IDynamicBuffer& GetOmniShadowBuffer() override;
			IDynamicBuffer& GetSpotShadowBuffer() override;

			IDynamicBuffer& GetGeobetryBatchBuffer() override;
			IDynamicBuffer& GetRawGeobetryBatchBuffer() override;

			IDynamicBuffer& GetFrustumBuffer() override;

			IDynamicBuffer& GetIndirectDrawBuffer() override;
			IDynamicBuffer& GetIndirectCountDrawBuffer() override;
			IDynamicBuffer& GetMeshCommandBuffer() override;

			IDynamicBuffer& GetCameraBuffer() override;
		private:
			void BuildGraph();
		private:
			filesystem::IVirtualFileSystemInterface* vfs{ nullptr };

			std::unique_ptr<gfx::Context> gfx_context{};
			std::unique_ptr<gfx::ResourceContext> resource_context{};
			std::unique_ptr<IRenderGraph> render_graph{};

			//GPUVector<FMaterial> materials{};

			std::unordered_map<uint32_t, std::unique_ptr<GeometryBuffer>> interleaved_geometry_buffers{};
			std::unordered_map<uint32_t, std::unique_ptr<GeometryBuffer>> deinterleaved_geometry_buffers{};

			uint32_t backbuffer_resource{ ~0u };

			// Gpu scene
			std::vector<std::unique_ptr<IRenderCamera>> render_cameras{};

			std::unique_ptr<IDynamicBuffer> node_buffer{};
			std::unique_ptr<IDynamicBuffer> matrices_buffer{};
			std::unique_ptr<IDynamicBuffer> mesh_buffer{};

			std::unique_ptr<IDynamicBuffer> material_buffer{};
			std::unique_ptr<IDynamicBuffer> light_buffer{};

			std::unique_ptr<IDynamicBuffer> cascaded_shadow_buffer{};
			std::unique_ptr<IDynamicBuffer> omni_shadow_buffer{};
			std::unique_ptr<IDynamicBuffer> spot_shadow_buffer{};

			std::unique_ptr<IDynamicBuffer> geometry_batches_buffer{};
			std::unique_ptr<IDynamicBuffer> raw_geometry_batches_buffer{};

			std::unique_ptr<IDynamicBuffer> frustum_buffer{};

			std::unique_ptr<IDynamicBuffer> indexed_draw_indirect_buffer{};
			std::unique_ptr<IDynamicBuffer> indirect_count_buffer{};
			std::unique_ptr<IDynamicBuffer> mesh_command_buffer{};

			std::unique_ptr<IDynamicBuffer> camera_buffer{};
		};
	}
}