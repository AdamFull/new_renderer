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
			void render(float dt) override;

			size_t createTexture(const std::string& name, const std::vector<uint8_t>& data, EMimeType mime) override;

			size_t createShader(const std::string& name, const FGraphicsShaderCreateInfo& create_info) override;
			size_t createShader(const std::string& name, const FComputeShaderCreateInfo& create_info) override;

			void updateSurfaceSize(int32_t width, int32_t height) override;
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

			std::vector<std::unique_ptr<IRenderingStep>> m_vRenderingSteps{};

			std::unique_ptr<IRenderContext> m_pRenderContext{};
			std::unique_ptr<CResourceLoader> m_pResourceLoader{};
			std::unique_ptr<CRenderScriptCore> m_pRenderScriptCore{};
		};
	}
}