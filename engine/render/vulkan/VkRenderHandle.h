#pragma once

#include <VkRenderContext.h>
#include <interface/ResourceContainer.h>
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
				FResourceView createTexture(const std::string& name, const FTextureCreateInfo& texture_ci, FTextureUploadInfo&& upload_info) override;
				FResourceView createRenderTarget(const std::string& name, const FTextureCreateInfo& texture_ci);
				FResourceView createRenderTargets(const std::string& name, const FTextureCreateInfo& texture_ci) override;

				FResourceView getResource(const char* name, uint32_t resource_type) override;

				FPoint2D<uint32_t> getRenderResolution() override;

				uint32_t getDepthFormat() override;
				uint32_t getApiFormat(uint32_t global_format) override;
				uint32_t getPresentFormat() override;

				void updateSurface(int32_t surface_width, int32_t surface_height) override;

				void beginFrameCommandBuffers() override;
				void endFrameCommandBuffers() override;
			protected:
				void processResourceUpload();


				FRenderApiErrorInfo createTexture(const FVkTextureCreateInfo& texture_ci, FVkTexture& texture);
				void destroyTexture(FVkTexture& texture);

				FRenderApiErrorInfo createShaderResources(const FVkShaderResourcesInfo& resources_info, FVkShader& shader);
				FRenderApiErrorInfo createShader(const FVkGraphicsShaderCreateInfo& shader_ci, FVkShader& shader);
				FRenderApiErrorInfo createShader(const FVkComputeShaderCreateInfo& shader_ci, FVkShader& shader);
				void destroyShader(FVkShader& shader);

				void updateShaderDescriptor(const FVkShaderDescriptorsUpdateInfo& update_info);
				void bindShaderDescriptors(const FVkCommandBuffer& command_buffer, const FVkShader& shader);

				void beginRendering(const FVkCommandBuffer command_buffer, const FVkRenderingInfo& rendering_info);
				void endRendering(const FVkCommandBuffer command_buffer, const FVkRenderingInfo& rendering_info);

				void processMemoryBarriers(const FVkCommandBuffer& command_buffer, const FVkTranslationInfo& translation_info);
				void processImageLayoutTranslation(const FVkCommandBuffer& command_buffer, const FVkImageTranslationInfo& image_translation_info);
				void textureLayoutTranslation(const FVkCommandBuffer& command_buffer, FVkTexture& texture, const FVkTextureTranslationInfo& translation_info);

				void copyBufferToTexture(const FVkCommandBuffer& command_buffer, const FVkBuffer& buffer, const FVkTexture& texture, const std::vector<vk::BufferImageCopy2KHR>& copy_regions);
				//void blitImage(const FVkCommandBuffer& command_buffer, );

				void draw();
			private:
				std::unique_ptr<CVkRenderContext> context{};
				std::unique_ptr<CResourceContainer> resources{};

				std::vector<FTextureUploadInfo> textureUploadInfos{};

				std::vector<FVkCommandBuffer> commandBuffers{};
				bool isFrameStarted{ false };

				int32_t surface_width{}, surface_height{};
			};
		}
	}
}