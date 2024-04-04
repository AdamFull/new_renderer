#pragma once

#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/interface/ResourceContainer.h>
#include "render_config/RenderConfig.h"
#include "ResourceLoader.h"
#include <memory>

namespace engine
{
	namespace render
	{
		class CRenderInterface : public IRenderInterface
		{
		public:
			CRenderInterface(filesystem::IVirtualFileSystemInterface* vfs);
			~CRenderInterface() override;

			void create(const FRenderInterfaceCreateInfo& render_interface_ci) override;
			void render(f32 dt) override;

			u64 createTexture(const stl::string& path) override;
			u64 loadTexture(const stl::string& path, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
			// TODO: add texture may be?

			u64 createShader(const stl::string& name, const FGraphicsShaderCreateInfo& create_info) override;
			u64 createShader(const stl::string& name, const FComputeShaderCreateInfo& create_info) override;

			u64 createVertexHandler(const stl::string& name, const FVertexHandlerCreateInfo& handler_ci) override;
			const stl::unique_ptr<IVertexHandler>& getVertexHandler(u64 id) override;

			void updateSurfaceSize(i32 width, i32 height) override;

			const stl::unique_ptr<FRenderDataPacket>& getRenderPacket() override;
			const stl::unique_ptr<IRenderGraph>& getRenderGraph() override;
			const stl::unique_ptr<IRenderWorld>& getRenderWorld() override;
			const stl::unique_ptr<IRenderContext>& getRenderContext() override;
		protected:
			template<class _Ty>
			u64 createShader(const FShaderUploadInfo& name_info, const _Ty& create_info)
			{
				stl::vector<FShaderCreationBlockInfo> shader_stages{};
				if (m_pResourceLoader->loadShaderData(name_info, shader_stages))
					return m_pRenderContext->createShader(name_info.originalName, create_info, shader_stages);

				log_error("Shader with name: \"{}\" was not found.", name_info.originalName);

				return ~0ull;
			}

			FShaderUploadInfo getShaderNameTag(const stl::string& name) override;

		private:
			filesystem::IVirtualFileSystemInterface* m_pVFS{ nullptr };

			stl::unique_ptr<FRenderDataPacket> m_pRenderPacket{};

			stl::unique_ptr<IRenderGraph> m_pRenderGraph{};
			stl::unique_ptr<IRenderWorld> m_pRenderWorld{};
			stl::unique_ptr<CResourceContainer> m_pResources{};
			stl::unique_ptr<IRenderContext> m_pRenderContext{};
			stl::unique_ptr<CResourceLoader> m_pResourceLoader{};
			stl::unique_ptr<CRenderScriptCore> m_pRenderScriptCore{};
		};
	}
}