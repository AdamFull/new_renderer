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

			u64 createTexture(const std::string& path) override;
			u64 loadTexture(const std::string& path, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
			// TODO: add texture may be?

			u64 createShader(const std::string& name, const FGraphicsShaderCreateInfo& create_info) override;
			u64 createShader(const std::string& name, const FComputeShaderCreateInfo& create_info) override;

			u64 createVertexHandler(const std::string& name, const FVertexHandlerCreateInfo& handler_ci) override;
			const std::unique_ptr<IVertexHandler>& getVertexHandler(u64 id) override;

			void updateSurfaceSize(i32 width, i32 height) override;

			const std::unique_ptr<FRenderDataPacket>& getRenderPacket() override;
			const std::unique_ptr<IRenderGraph>& getRenderGraph() override;
			const std::unique_ptr<IRenderWorld>& getRenderWorld() override;
			const std::unique_ptr<IRenderContext>& getRenderContext() override;
		protected:
			template<class _Ty>
			u64 createShader(const FShaderUploadInfo& name_info, const _Ty& create_info)
			{
				std::vector<FShaderCreationBlockInfo> shader_stages{};
				if (m_pResourceLoader->loadShaderData(name_info, shader_stages))
					return m_pRenderContext->createShader(name_info.originalName, create_info, shader_stages);

				return ~0ull;
			}

			FShaderUploadInfo getShaderNameTag(const std::string& name) override;

		private:
			filesystem::IVirtualFileSystemInterface* m_pVFS{ nullptr };

			std::unique_ptr<FRenderDataPacket> m_pRenderPacket{};

			std::unique_ptr<IRenderGraph> m_pRenderGraph{};
			std::unique_ptr<IRenderWorld> m_pRenderWorld{};
			std::unique_ptr<CResourceContainer> m_pResources{};
			std::unique_ptr<IRenderContext> m_pRenderContext{};
			std::unique_ptr<CResourceLoader> m_pResourceLoader{};
			std::unique_ptr<CRenderScriptCore> m_pRenderScriptCore{};
		};
	}
}