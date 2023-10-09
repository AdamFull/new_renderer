#pragma once

#include <VkRenderContext.h>
#include <VkResourceContext.h>
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

				void updateSurface(int32_t surface_width, int32_t surface_height) override;

				void beginFrameCommandBuffers() override;
				void endFrameCommandBuffers() override;
			protected:
				FRenderApiErrorInfo createTexture(const FVkTextureCreateInfo& texture_ci, FVkTexture& texture);
				void destroyTexture(FVkTexture& texture);

				FRenderApiErrorInfo createShader(const FVkGraphicsShaderCreateInfo& shader_ci, FVkShader& shader);
				FRenderApiErrorInfo createShader(const FVkComputeShaderCreateInfo& shader_ci, FVkShader& shader);
				void destroyShader(FVkShader& shader);

				void updateDescriptors(const FVkDescriptorBufferUpdateInfo& update_info, FVkShader& shader)

				void beginRendering(const FVkCommandBuffer command_buffer, const FVkRenderingInfo& rendering_info);
				void endRendering(const FVkCommandBuffer command_buffer, const FVkRenderingInfo& rendering_info);

				void processMemoryBarriers(const FVkCommandBuffer& command_buffer, const FVkTranslationInfo& translation_info);
				void processImageLayoutTranslation(const FVkCommandBuffer& command_buffer, const FVkImageTranslationInfo& image_translation_info);
				void textureLayoutTranslation(const FVkCommandBuffer& command_buffer, FVkTexture& texture, const FVkTextureTranslationInfo& translation_info);

				void draw();
			private:
				std::unique_ptr<CVkRenderContext> context{};
				std::unique_ptr<CVkResourceContext> resources{};

				FVkCommandBuffer commandBuffer{};
				bool isFrameStarted{ false };

				int32_t surface_width{}, surface_height{};

				std::vector<resource_handle_t> swapchainTargets{};

				// TEMPORARY. FOR TESTING
				resource_handle_t testShader{};
			};
		}
	}
}