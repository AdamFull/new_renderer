#pragma once

#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/interface/render/RenderInterface.h>
#include <engine/interface/ResourceContainer.h>
#include <util/Logger.h>
#include <memory>

namespace engine
{
	namespace gfx
	{
		class GFXContext;
	}

	namespace render
	{
		using resource_container_t = CResourceContainer<IBufferObject, ITextureObject, IShaderObject, IVertexHandler>;

		class CRenderInterface : public IRenderInterface
		{
		public:
			CRenderInterface(filesystem::IVirtualFileSystemInterface* vfs);
			~CRenderInterface() override;

			void PreInitialize(const FRenderInterfaceCreateInfo& render_interface_ci) override;
			void Initialize() override;
			void PostInitialize() override;

			void PreUpdate(f32 dt) override;
			void Update(f32 dt) override;
			void PostUpdate(f32 dt) override;

			void PreRender() override;
			void Render() override;
			void PostRender() override;

			FRHIHandle CreateTextureObject(const std::string& path) override;
			FRHIHandle CreateTextureObject(const std::string& name, const FTextureCreateInfo& texture_ci) override;
			FRHIHandle CreateTextureObject(const std::string& name, const FTextureCreateInfo& texture_ci, const FTextureUploadInfo& upload_info) override;
			void DestroyTextureObject(FRHIHandle handle) override;
			const std::unique_ptr<ITextureObject>& GetTextureObject(FRHIHandle handle) override;
			const std::unique_ptr<ITextureObject>& GetTextureObject(const char* name) override;

			FRHIHandle CreateBufferObject(const std::string& name, const FBufferCreateInfo& buffer_ci) override;
			void DestroyBufferObject(FRHIHandle handle) override;
			const std::unique_ptr<IBufferObject>& GetBufferObject(FRHIHandle handle) override;
			const std::unique_ptr<IBufferObject>& GetBufferObject(const char* name) override;

			FRHIHandle CreateShaderObject(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci) override;
			FRHIHandle CreateShaderObject(const std::string& name, const FComputeShaderCreateInfo& shader_ci) override;
			void DestroyShaderObject(FRHIHandle handle) override;
			const std::unique_ptr<IShaderObject>& GetShaderObject(FRHIHandle id) override;
			const std::unique_ptr<IShaderObject>& GetShaderObject(const char* name) override;

			FRHIHandle CreateVertexHandler(const std::string& name, const FVertexHandlerCreateInfo& handler_ci) override;
			void DestroyVertexHandler(FRHIHandle handle) override;
			const std::unique_ptr<IVertexHandler>& GetVertexHandler(FRHIHandle id) override;

			void UpdateSurfaceSize(i32 width, i32 height) override;

			FShaderUploadInfo GetShaderNameTag(const std::string& name) override;

			const std::unique_ptr<FRenderDataPacket>& GetRenderPacket() override;
			const std::unique_ptr<IRenderGraph>& GetRenderGraph() override;
			const std::unique_ptr<IRenderWorld>& GetRenderWorld() override;
			const std::unique_ptr<IRenderBackend>& GetRenderBackend() override;
			const std::unique_ptr<IResourceLoader>& GetResourceLoader() override;
		protected:
			FTextureCreateInfo ApplyTextureExtent(const FTextureCreateInfo& texture_ci);
		private:
			filesystem::IVirtualFileSystemInterface* m_pVFS{ nullptr };

			u32 m_frameCounter{ 0u };

			std::unique_ptr<gfx::GFXContext> gfx_context{};

			std::unique_ptr<FRenderDataPacket> m_pRenderPacket{};

			std::unique_ptr<IRenderGraph> m_pRenderGraph{};
			std::unique_ptr<IRenderWorld> m_pRenderWorld{};
			std::unique_ptr<resource_container_t> render_resources{};
			std::unique_ptr<IRenderBackend> m_pRenderContext{};
			std::unique_ptr<IResourceLoader> m_pResourceLoader{};
		};
	}
}