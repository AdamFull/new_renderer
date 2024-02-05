#pragma once

#include "VkRenderDevice.h"
#include <engine/interface/ResourceContainer.h>
#include <engine/interface/render/RenderInterface.h>

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
				void createGlobalDescriptor(const FGlobalUniformCreateInfo& uniform_ci) override;
				void updateGlobalDescriptorData(const FGlobalUniformUpdateInfo& data_view) override;

				u64 createTexture(const stl::string& name, const FTextureCreateInfo& texture_ci, FTextureUploadInfo&& upload_info) override;
				u64 getTextureId(const stl::string& name) override;
				u64 createRenderTarget(const stl::string& name, const FTextureCreateInfo& texture_ci);
				u64 createRenderTargets(const stl::string& name, const FTextureCreateInfo& texture_ci) override;
				u64 getTextureIndex(u64 texture_id, u32 layer = 0u, u32 mip = 0u) override;
				const FTextureCreateInfo& getTextureInfo(u64 texture_id) override;
				void destroyTexture(u64 resource_id) override;


				u64 createShader(const stl::string& name, const FGraphicsShaderCreateInfo& shader_ci, const  stl::vector<FShaderCreationBlockInfo>& shader_stages) override;
				u64 createShader(const stl::string& name, const FComputeShaderCreateInfo& shader_ci, const  stl::vector<FShaderCreationBlockInfo>& shader_stages) override;
				FShaderReflectionData getShaderReflection(const u64& shader_view) override;
				void updateShaderResources(const FShaderResourceCommit& shader_resources, EShaderResourcesUpdateFlags flags = EShaderResourcesUpdateFlagBits::eAll) override;
				void flushConstantRanges(u64 shader_id, const stl::unordered_map<u64, FUniformDataView>& constants, EQueueTypeFlagBits queue_type) override;
				void bindShader(u64 shader_id, EQueueTypeFlagBits queue_type) override;
				void destroyShader(u64 resource_id) override;


				u64 createBuffer(const stl::string& name, const FBufferCreateInfo& buffer_ci) override;
				void uploadDataToBuffer(const FBufferUploadInfo& upload_info) override;
				FBufferView getBufferView(u64 buffer_id) override;
				void bindVertexBuffers(const FVertexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type) override;
				void bindIndexBuffer(const FIndexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type) override;
				void mapBufferMemory(u64 buffer_id, void*& memory) override;
				void unmapBufferMemory(u64 buffer_id) override;
				void flushBuffer(u64 buffer_id, u64 size, u64 offset) override;
				void destroyBuffer(u64 resource_id) override;
			

				void pushG2CBarriers(EQueueTypeFlagBits queue_type, const stl::vector<u64>& texture_ids) override;
				void pushC2CBarrier(EQueueTypeFlagBits queue_type, const stl::vector<u64>& texture_ids) override;
				void pushC2GBarrier(EQueueTypeFlagBits queue_type, const stl::vector<u64>& texture_ids) override;

				void processResourceBarriers(EQueueTypeFlagBits queue, const stl::vector<FResourceBarrierInfo>& barriers) override;

				void beginRendering(const FBeginRendering2Info& rendering_info) override;
				void endRendering(EQueueTypeFlagBits queue) override;

				void setViewport(f32 width, f32 height, bool flip_viewport = false) override;
				void setScissorRect(i32 offset_x, i32 offset_y, u32 width, u32 height) override;

				void draw(EQueueTypeFlagBits queue_type, const FGeometryDrawInfo& draw_info) override;
				void draw(EQueueTypeFlagBits queue_type, const FIndirectGeometryDrawInfo& draw_info) override;

				void dispatch(EQueueTypeFlagBits queue_type, const FDispatch2Info& dispatch_group) override;
				void dispatch(EQueueTypeFlagBits queue_type, const FDispatchIndirectInfo& dispatch_info) override;

				u64 getTextureResource(const char* name) override;
				u64 getShaderResource(const char* name) override;

				FPoint2D<u32> getRenderResolution() const override;
				FRect2D<f32> getRenderRectFromRT(u64 texture_id) override;
				FRect2D<f32> getScreenRenderRect() override;

				u32 getDepthFormat(bool depth_only = false) override;
				u32 getApiFormat(EApiDataFormat global_format) override;
				u32 getPresentFormat() override;

				void updateSurface(i32 surface_width, i32 surface_height) override;

				FVkCommandBuffer& createCommandBuffer(const FVkCommandBufferCreateInfo& command_buffer_ci, stl::thread::id thread_id = stl::this_thread::get_id());
				FVkCommandBuffer& getLastCommandBuffer(EQueueTypeFlagBits queue_type, stl::thread::id thread_id = stl::this_thread::get_id());
				FVkCommandBuffer& getCommandBuffer(EQueueTypeFlagBits queue_type, u64 index, stl::thread::id thread_id = stl::this_thread::get_id());
				stl::vector<FVkCommandBuffer>& getCommandBuffers(EQueueTypeFlagBits queue_type, stl::thread::id thread_id = stl::this_thread::get_id());
				void destroyCommandBuffer(EQueueTypeFlagBits queue_type, stl::thread::id thread_id = stl::this_thread::get_id());

				void beginFrameCommandBuffers() override;
				void endFrameCommandBuffers() override;
				
				bool isSwapChainFinished() override;

				void beginDebugLabel(const stl::string& label, EQueueTypeFlagBits queue_type, u32 color = 0xffffffff) override;
				void insertDebugLabel(const stl::string& label, EQueueTypeFlagBits queue_type, u32 color = 0xffffffff) override;
				void endDebugLabel(EQueueTypeFlagBits queue_type) override;
			protected:
				void destroyThreadContext(FVkThreadContext& context);
				void processResourceBarriers();
				void processResourceUpload();
				void processResourceUtilization();

				FRenderApiErrorInfo createTexture(const FVkTextureCreateInfo& texture_ci, FVkTexture& texture);
				void destroyTexture(FVkTexture& texture);

				FRenderApiErrorInfo createShaderResources(const FVkShaderResourcesInfo& resources_info, FVkShader& shader);
				void destroyShader(FVkShader& shader);

				void bindShaderDescriptors(const FVkCommandBuffer& command_buffer, const FVkShader& shader);

				void processMemoryBarriers(const FVkCommandBuffer& command_buffer, const FVkTranslationInfo& translation_info);
				void processImageLayoutTranslation(const FVkCommandBuffer& command_buffer, const FVkImageTranslationInfo& image_translation_info);
				void textureLayoutTranslation(const FVkCommandBuffer& command_buffer, FVkTexture& texture, const FVkTextureTranslationInfo& translation_info);

				void copyBufferToTexture(const FVkCommandBuffer& command_buffer, const FVkBuffer& buffer, const FVkTexture& texture, const stl::vector<vk::BufferImageCopy2KHR>& copy_regions);
				//void blitImage(const FVkCommandBuffer& command_buffer, );
			private:
				stl::unique_ptr<CVkRenderDevice> m_pDevice{};
				stl::unique_ptr<CResourceContainer> m_pResources{};

				stl::vector<FTextureUploadInfo> m_textureUploadInfos{};
				stl::vector<FBufferUploadInfo> m_bufferUploadInfos{};
				stl::vector<FVkTextureLayoutChangeInfo> m_textureLayoutInfos{};

				stl::vector<u64> m_textureIndexRemap{};

				stl::queue<FAPIObjectDestroyInfo> m_destroyResource{};

				u64 m_indirectDispatchBuffer{ ~0ull };
				u64 m_dispatchBufferOffset{ 0ull };

				// NOTE: global descriptor uses set 0 and binding 0 and 1 (0 is for global uniform, 1 is for texture array)
				FVkGlobalDescriptor m_globalDescriptor{};
				stl::unordered_map<u32, utl::unique_queue<u64>> m_textureToUpdate{};

				stl::unordered_map<stl::thread::id, FVkThreadContext> m_perThreadContext{};
				bool m_bIsFrameStarted{ false };

				u64 m_utilizationCounter{ 0ull };

				i32 m_iSurfaceWidth{}, m_iSurfaceHeight{};
			};
		}
	}
}