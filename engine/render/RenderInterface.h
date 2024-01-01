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
			void render(float dt) override;

			size_t createTexture(const std::string& path) override;
			// TODO: add texture may be?

			size_t createShader(const std::string& name, const FGraphicsShaderCreateInfo& create_info) override;
			size_t createShader(const std::string& name, const FComputeShaderCreateInfo& create_info) override;

			size_t createVertexHandler(const std::string& name, const FVertexHandlerCreateInfo& handler_ci) override;
			const std::unique_ptr<IVertexHandler>& getVertexHandler(size_t id) override;

			void updateSurfaceSize(int32_t width, int32_t height) override;

			const std::unique_ptr<FRenderDataPacket>& getRenderPacket() override;
			const std::unique_ptr<IRenderWorld>& getRenderWorld() override;

			IRenderContext* getRenderContext() override;
		protected:
			template<class _Ty>
			size_t createShader(const FShaderUploadInfo& name_info, const _Ty& create_info)
			{
				std::vector<FShaderCreationBlockInfo> shader_stages{};
				if (m_pResourceLoader->loadShaderData(name_info, shader_stages))
					return m_pRenderContext->createShader(name_info.originalName, create_info, shader_stages);

				return ~0ull;
			}

			FShaderUploadInfo getShaderNameTag(const std::string& name) override;
			void createRendering(const FRenderingStepCreateInfo& rendering_ci) override;

		private:
			filesystem::IVirtualFileSystemInterface* m_pVFS{ nullptr };

			std::vector<std::unique_ptr<IRenderGraphStep>> m_vRenderingSteps{};

			// TODO: add map with entity id and mesh data

			std::vector<size_t> m_matPassRTs{};
			size_t m_matPassDepth{ ~0ull };
			size_t m_matPassStencil{ ~0ull };

			std::unique_ptr<FRenderDataPacket> m_pRenderPacket{};

			std::unique_ptr<IRenderWorld> m_pRenderWorld{};
			std::unique_ptr<CResourceContainer> m_pResources{};
			std::unique_ptr<IRenderContext> m_pRenderContext{};
			std::unique_ptr<CResourceLoader> m_pResourceLoader{};
			std::unique_ptr<CRenderScriptCore> m_pRenderScriptCore{};
		};
	}
}