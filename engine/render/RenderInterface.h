#pragma once

#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/interface/render/RenderInterface.h>
#include <engine/interface/ResourceContainer.h>
#include <logger/logger.h>
#include <memory>

namespace engine
{
	namespace render
	{
		using resource_container_t = CResourceContainer<IBufferObject, ITextureObject, IShaderObject, IVertexHandler>;

		class CRenderInterface : public IRenderInterface
		{
		public:
			CRenderInterface(filesystem::IVirtualFileSystemInterface* vfs);
			~CRenderInterface() override;

			void Create(const FRenderInterfaceCreateInfo& render_interface_ci) override;
			void PostLoad() override;
			void Render(f32 dt) override;

			FAPIHandle CreateTexture(const std::string& path) override;
			FAPIHandle LoadTexture(const std::string& path, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
			// TODO: add texture may be?

			FAPIHandle CreateShader(const std::string& name, const FGraphicsShaderCreateInfo& create_info) override;
			FAPIHandle CreateShader(const std::string& name, const FComputeShaderCreateInfo& create_info) override;
			const std::unique_ptr<IShaderObject>& GetShaderObject(FAPIHandle id) override;

			u64 CreateVertexHandler(const std::string& name, const FVertexHandlerCreateInfo& handler_ci) override;
			const std::unique_ptr<IVertexHandler>& GetVertexHandler(u64 id) override;

			void UpdateSurfaceSize(i32 width, i32 height) override;

			FShaderUploadInfo GetShaderNameTag(const std::string& name) override;

			const std::unique_ptr<FRenderDataPacket>& GetRenderPacket() override;
			const std::unique_ptr<IRenderGraph>& GetRenderGraph() override;
			const std::unique_ptr<IRenderWorld>& GetRenderWorld() override;
			const std::unique_ptr<IRenderContext>& GetRenderContext() override;
			const std::unique_ptr<IResourceLoader>& GetResourceLoader() override;
		protected:
			template<class _Ty>
			FAPIHandle CreateShader(const FShaderUploadInfo& name_info, const _Ty& create_info)
			{
				utl::vector<FShaderCreationBlockInfo> shader_stages{};
				if (m_pResourceLoader->LoadShaderData(name_info, shader_stages))
					return m_pRenderContext->CreateShader(name_info.originalName, create_info, shader_stages);

				log_error("Shader with name: \"{}\" was not found.", name_info.originalName);

				return FAPIHandle(INVALID_HANDLE);
			}

		private:
			filesystem::IVirtualFileSystemInterface* m_pVFS{ nullptr };

			u32 m_frameCounter{ 0u };

			std::unique_ptr<FRenderDataPacket> m_pRenderPacket{};

			std::unique_ptr<IRenderGraph> m_pRenderGraph{};
			std::unique_ptr<IRenderWorld> m_pRenderWorld{};
			std::unique_ptr<resource_container_t> m_pResources{};
			std::unique_ptr<IRenderContext> m_pRenderContext{};
			std::unique_ptr<IResourceLoader> m_pResourceLoader{};
		};
	}
}