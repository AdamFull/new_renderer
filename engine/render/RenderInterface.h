#pragma once

#include <interface/filesystem/VirtualFileSystemInterface.h>
#include <render_config/RenderConfig.h>
#include <ResourceLoader.h>
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

			FResourceView createTexture(const std::string& name, const std::vector<uint8_t>& data, EMimeType mime) override;

			FResourceView createShader(const FShaderNameLoadInfo& name_info, const FGraphicsShaderCreateInfo& create_info) override;
			FResourceView createShader(const FShaderNameLoadInfo& name_info, const FComputeShaderCreateInfo& create_info) override;

			void updateSurfaceSize(int32_t width, int32_t height) override;

			void beginFrame() override;
			void endFrame() override;
		protected:
			template<class _Ty>
			FResourceView createShader(const FShaderNameLoadInfo& name_info, const _Ty& create_info)
			{
				std::vector<FShaderCreationBlockInfo> shader_stages{};
				if (m_pResourceLoader->loadShaderData(name_info, shader_stages))
					return m_pRenderHandle->createShader(name_info.originalName, create_info, shader_stages);

				return {};
			}

			FShaderNameLoadInfo getShaderNameTag(const std::string& name) override;
			void createRendering(const FRenderingCreateInfo& rendering_ci) override;

		private:
			filesystem::IVirtualFileSystemInterface* m_pVFS{ nullptr };

			std::vector<std::unique_ptr<CDynamicRenderStage>> m_vRenderStages{};

			std::unique_ptr<IRenderHandleInterface> m_pRenderHandle{};
			std::unique_ptr<CResourceLoader> m_pResourceLoader{};
			std::unique_ptr<CRenderScriptCore> m_pRenderScriptCore{};
		};
	}
}