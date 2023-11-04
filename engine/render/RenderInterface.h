#pragma once

#include <RenderConfig.h>
#include <ResourceLoader.h>
#include <memory>

#include "ShaderObject.h"

namespace engine
{
	namespace render
	{
		class CRenderInterface : public IRenderInterface
		{
		public:
			~CRenderInterface() override;

			void create(const FRenderInterfaceCreateInfo& render_interface_ci) override;

			void createRendererBehavior(const std::vector<uint8_t>& script) override;

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
				if (resourceLoader->loadShaderData(name_info, shader_stages))
					return renderHandle->createShader(name_info.originalName, create_info, shader_stages);

				return {};
			}

			template<class _Ty>
			std::unique_ptr<CShaderObject> createShaderObject(const FShaderNameLoadInfo& name_info, const _Ty& create_info)
			{
				std::vector<FShaderCreationBlockInfo> shader_stages{};
				if (resourceLoader->loadShaderData(name_info, shader_stages))
				{
					auto shader_object = std::make_unique<CShaderObject>();
					shader_object->shaderView = renderHandle->createShader(name_info.originalName, create_info, shader_stages);

					auto reflection_data = renderHandle->getShaderReflection(shader_object->shaderView);

					for (auto& [id, uniform_block] : reflection_data.uniformBlocks)
					{
						auto uniform_handle = std::make_unique<CUniformHandle>();
						uniform_handle->create(uniform_block);
						shader_object->uniformBlocks[id] = std::move(uniform_handle);
					}

					for (auto& [id, push_block] : reflection_data.pushBlocks)
					{
						auto uniform_handle = std::make_unique<CUniformHandle>();
						uniform_handle->create(push_block);
						shader_object->uniformBlocks[id] = std::move(uniform_handle);
					}

					return shader_object;
				}

				return nullptr;
			}

			FShaderNameLoadInfo getShaderNameTag(const std::string& name) override;
			void createRendering(const FRenderingCreateInfo& rendering_ci) override;

		private:
			std::unique_ptr<IRenderHandleInterface> renderHandle{};
			std::unique_ptr<CResourceLoader> resourceLoader{};
			std::unique_ptr<CRenderConfig> renderConfig{};
		};
	}
}