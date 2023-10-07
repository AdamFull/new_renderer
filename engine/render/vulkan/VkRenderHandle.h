#pragma once

#include "VkRenderContext.h"
#include <interface/RenderInterface.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class CVkRenderHandle : public IRenderHandleInterface
			{
			public:
				~CVkRenderHandle() override;

				void create(const FRenderContextCreateInfo& context_ci) override;

				void syncGPUIdle() override;
				void updateSurface(int32_t surface_width, int32_t surface_height) override;

				void beginFrameCommandBuffers() override;
				void endFrameCommandBuffers() override;
			protected:
				FRenderApiErrorInfo createTexture(const FVkTextureCreateInfo& texture_ci, FVkTexture& texture);
				void destroyTexture(FVkTexture& texture);

				FRenderApiErrorInfo createShader(const FVkGraphicsShaderCreateInfo& shader_ci, FVkShader& shader);
				FRenderApiErrorInfo createShader(const FVkComputeShaderCreateInfo& shader_ci, FVkShader& shader);
				void destroyShader(FVkShader& shader);

				void uploadTextureData();
			private:
				std::unique_ptr<CVkRenderContext> context{};

				FVkCommandBuffer commandBuffer{};
				bool isFrameStarted{ false };

				int32_t surface_width{}, surface_height{};
			};
		}
	}
}