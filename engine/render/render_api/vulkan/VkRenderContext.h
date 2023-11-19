#pragma once

#include <VkRenderDevice.h>
#include <interface/ResourceContainer.h>
#include <interface/render/RenderInterface.h>

#include <ucontainer.hpp>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class CVkRenderContext : public IRenderContext
			{
			public:
				~CVkRenderContext() override;

				void create(const FRenderContextCreateInfo& context_ci) override;
				size_t createTexture(const std::string& name, const FTextureCreateInfo& texture_ci, FTextureUploadInfo&& upload_info) override;
				size_t createRenderTarget(const std::string& name, const FTextureCreateInfo& texture_ci);
				size_t createRenderTargets(const std::string& name, const FTextureCreateInfo& texture_ci) override;

				size_t createShader(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci, const  std::vector<FShaderCreationBlockInfo>& shader_stages) override;
				size_t createShader(const std::string& name, const FComputeShaderCreateInfo& shader_ci, const  std::vector<FShaderCreationBlockInfo>& shader_stages) override;

				size_t createBuffer(const std::string& name, const FBufferCreateInfo& buffer_ci) override;
				void uploadDataToBuffer(const FBufferUploadInfo& upload_info) override;

				void destroyTexture(size_t resource_id) override;
				void destroyShader(size_t resource_id) override;

				void beginRendering(const FBeginRenderingInfo& rendering_info) override;
				void endRendering(const FBeginRenderingInfo& rendering_info) override;

				void setViewport(float width, float height, bool flip_viewport = false) override;
				void setScissorRect(int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height) override;

				void bindVertexBuffers(const FVertexBufferBindingInfo& binding_info, uint32_t queue_type) override;
				void bindIndexBuffer(const FIndexBufferBindingInfo& binding_info, uint32_t queue_type) override;

				void bindShader(size_t shader_id, uint32_t queue_type) override;

				FShaderReflectionData getShaderReflection(const size_t& shader_view) override;
				void updateShaderResources(const FShaderResourceCommit& shader_resources, uint32_t flags = SHADER_RESOURCES_UPDATE_ALL_FLAG_BIT) override;
				void flushConstantRanges(size_t shader_id, const std::unordered_map<size_t, FUniformDataView>& constants, uint32_t queue_type) override;

				size_t getTextureResource(const char* name) override;
				size_t getShaderResource(const char* name) override;

				FPoint2D<uint32_t> getRenderResolution() override;

				uint32_t getDepthFormat() override;
				uint32_t getApiFormat(uint32_t global_format) override;
				uint32_t getPresentFormat() override;

				void updateSurface(int32_t surface_width, int32_t surface_height) override;

				FVkCommandBuffer& createCommandBuffer(const FVkCommandBufferCreateInfo& command_buffer_ci, std::thread::id thread_id = std::this_thread::get_id());
				FVkCommandBuffer& getLastCommandBuffer(uint32_t queue_type, std::thread::id thread_id = std::this_thread::get_id());
				FVkCommandBuffer& getCommandBuffer(uint32_t queue_type, size_t index, std::thread::id thread_id = std::this_thread::get_id());
				std::vector<FVkCommandBuffer>& getCommandBuffers(uint32_t queue_type, std::thread::id thread_id = std::this_thread::get_id());

				void beginFrameCommandBuffers() override;
				void endFrameCommandBuffers() override;
			protected:
				void createGlobalDescriptor();
				void updateGlobalDescriptorData();

				void processResourceUpload();

				FRenderApiErrorInfo createTexture(const FVkTextureCreateInfo& texture_ci, FVkTexture& texture);
				void destroyTexture(FVkTexture& texture);

				FRenderApiErrorInfo createShaderResources(const FVkShaderResourcesInfo& resources_info, FVkShader& shader);
				void destroyShader(FVkShader& shader);

				void bindShaderDescriptors(const FVkCommandBuffer& command_buffer, const FVkShader& shader);

				void processMemoryBarriers(const FVkCommandBuffer& command_buffer, const FVkTranslationInfo& translation_info);
				void processImageLayoutTranslation(const FVkCommandBuffer& command_buffer, const FVkImageTranslationInfo& image_translation_info);
				void textureLayoutTranslation(const FVkCommandBuffer& command_buffer, FVkTexture& texture, const FVkTextureTranslationInfo& translation_info);

				void copyBufferToTexture(const FVkCommandBuffer& command_buffer, const FVkBuffer& buffer, const FVkTexture& texture, const std::vector<vk::BufferImageCopy2KHR>& copy_regions);
				//void blitImage(const FVkCommandBuffer& command_buffer, );
			private:
				std::unique_ptr<CVkRenderDevice> m_pContext{};
				std::unique_ptr<CResourceContainer> m_pResources{};

				std::vector<FTextureUploadInfo> m_textureUploadInfos{};
				std::vector<FBufferUploadInfo> m_bufferUploadInfos{};

				// NOTE: global descriptor uses set 0 and binding 0 and 1 (0 is for global uniform, 1 is for texture array)
				FVkGlobalDescriptor m_globalDescriptor{};
				std::unordered_map<uint32_t, utl::unique_queue<size_t>> m_textureToUpdate{};

				std::unordered_map<FVkThreadKeyResolver, std::vector<FVkCommandBuffer>> m_commandBuffers{};
				bool m_bIsFrameStarted{ false };

				int32_t m_iSurfaceWidth{}, m_iSurfaceHeight{};
			};
		}
	}
}