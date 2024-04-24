#pragma once

#include "VkRenderDevice.h"
#include <engine/interface/ResourceContainer.h>
#include <engine/interface/render/RenderInterface.h>
#include <container/unique_queue.h>

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
				const utl::vector<uint32_t>& getGlobalTextureRemap() override;

				u64 createTexture(const std::string& name, const FTextureCreateInfo& texture_ci, FTextureUploadInfo&& upload_info) override;
				u64 getTextureId(const std::string& name) override;
				u64 createRenderTarget(const std::string& name, const FTextureCreateInfo& texture_ci) override;
				const FTextureCreateInfo& getTextureInfo(u64 texture_id) override;
				void destroyTexture(u64 resource_id) override;


				u64 createShader(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci, const  utl::vector<FShaderCreationBlockInfo>& shader_stages) override;
				u64 createShader(const std::string& name, const FComputeShaderCreateInfo& shader_ci, const  utl::vector<FShaderCreationBlockInfo>& shader_stages) override;
				u64 createShader(const std::string& name, const FReyTracingShaderCreateInfo& shader_ci, const  utl::vector<FShaderCreationBlockInfo>& shader_stages) override;
				FShaderReflectionData getShaderReflection(const u64& shader_view) override;
				void updateShaderResources(const FShaderResourceCommit& shader_resources, EShaderResourcesUpdateFlags flags = EShaderResourcesUpdateFlagBits::eAll) override;
				void flushConstantRanges(u64 shader_id, const utl::hash_map<u64, FUniformDataView>& constants, EQueueTypeFlagBits queue_type) override;
				void bindShader(u64 shader_id, EQueueTypeFlagBits queue_type) override;
				void destroyShader(u64 resource_id) override;


				u64 createBuffer(const std::string& name, const FBufferCreateInfo& buffer_ci) override;
				void updateBuffer(const std::string& name, u64 buffer_id, const FBufferCreateInfo& buffer_ci) override;
				void uploadDataToBuffer(const FBufferUploadInfo& upload_info) override;
				FBufferView getBufferView(u64 buffer_id) override;
				void bindVertexBuffers(const FVertexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type) override;
				void bindIndexBuffer(const FIndexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type) override;
				void mapBufferMemory(u64 buffer_id, void*& memory) override;
				void unmapBufferMemory(u64 buffer_id) override;
				void flushBuffer(u64 buffer_id, u64 size, u64 offset) override;
				void destroyBuffer(u64 resource_id) override;
			

				void pushG2CBarriers(EQueueTypeFlagBits queue_type, const utl::vector<u64>& texture_ids) override;
				void pushC2CBarrier(EQueueTypeFlagBits queue_type) override;
				void pushC2GBarrier(EQueueTypeFlagBits queue_type, const utl::vector<u64>& texture_ids) override;

				void processResourceBarriers(EQueueTypeFlagBits queue, const utl::vector<FResourceBarrierInfo>& barriers) override;

				void beginTimestampZone(EQueueTypeFlagBits queue) override;
				void endTimestampZone(EQueueTypeFlagBits queue) override;

				void beginRendering(const FBeginRendering2Info& rendering_info) override;
				void endRendering(EQueueTypeFlagBits queue) override;

				void beginCompute(const FBeginComputeInfo& compute_info) override;
				void endCompute(EQueueTypeFlagBits queue) override;

				void setViewport(f32 width, f32 height, bool flip_viewport = false) override;
				void setScissorRect(i32 offset_x, i32 offset_y, u32 width, u32 height) override;

				void draw(EQueueTypeFlagBits queue_type, const FGeometryDrawInfo& draw_info) override;
				void draw(EQueueTypeFlagBits queue_type, const FIndirectGeometryDrawInfo& draw_info) override;

				void dispatch(EQueueTypeFlagBits queue_type, const FDispatch2Info& dispatch_group, u64 subgroup_index) override;
				void dispatch(EQueueTypeFlagBits queue_type, const FDispatchIndirectInfo& dispatch_info) override;

				FDeviceLimitations getDeviceLimitations() override;

				const FRenderFrameStats& getRenderFrameStats() const override;

				u64 getTextureResource(const char* name) override;
				u64 getShaderResource(const char* name) override;

				FPoint2D<u32> getRenderResolution() const override;
				FRect2D<f32> getRenderRectFromRT(u64 texture_id) override;
				FRect2D<f32> getScreenRenderRect() override;

				u32 getDepthFormat(bool depth_only = false) override;
				u32 getApiFormat(EApiDataFormat global_format) override;
				u32 getPresentFormat() override;

				void updateSurface(i32 surface_width, i32 surface_height) override;

				FVkCommandBuffer& createCommandBuffer(const FVkCommandBufferCreateInfo& command_buffer_ci, std::thread::id thread_id = std::this_thread::get_id());
				FVkCommandBuffer& getLastCommandBuffer(EQueueTypeFlagBits queue_type, std::thread::id thread_id = std::this_thread::get_id());
				FVkCommandBuffer& getCommandBuffer(EQueueTypeFlagBits queue_type, u64 index, std::thread::id thread_id = std::this_thread::get_id());
				utl::vector<FVkCommandBuffer>& getCommandBuffers(EQueueTypeFlagBits queue_type, std::thread::id thread_id = std::this_thread::get_id());
				void destroyCommandBuffer(EQueueTypeFlagBits queue_type, std::thread::id thread_id = std::this_thread::get_id());

				void beginFrame() override;
				void endFrame() override;
				
				bool isSwapChainFinished() override;

				void beginDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color = 0xffffffff) override;
				void insertDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color = 0xffffffff) override;
				void endDebugLabel(EQueueTypeFlagBits queue_type) override;
			protected:
				void destroyThreadContext(FVkThreadContext& context);
				void processResourceBarriers();
				void processResourceUpload();
				void processResourceUtilization();
				void updateBackbuffer();

				FVkBufferCreateInfo makeBufferCreateInfo(const std::string& name, const FBufferCreateInfo& buffer_ci);

				FRenderApiErrorInfo createTexture(const FVkTextureCreateInfo& texture_ci, FVkTexture& texture);
				void destroyTexture(FVkTexture& texture);

				FRenderApiErrorInfo createShaderResources(const FVkShaderResourcesInfo& resources_info, FVkShader& shader);
				void destroyShader(FVkShader& shader);

				void bindShaderDescriptors(const FVkCommandBuffer& command_buffer, const FVkShader& shader);

				void processMemoryBarriers(const FVkCommandBuffer& command_buffer, const FVkTranslationInfo& translation_info);
				void processImageLayoutTranslation(const FVkCommandBuffer& command_buffer, const FVkImageTranslationInfo& image_translation_info);
				void textureLayoutTranslation(const FVkCommandBuffer& command_buffer, FVkTexture& texture, const FVkTextureTranslationInfo& translation_info);

				void copyBufferToTexture(const FVkCommandBuffer& command_buffer, const FVkBuffer& buffer, const FVkTexture& texture, const utl::vector<vk::BufferImageCopy2KHR>& copy_regions);
				//void blitImage(const FVkCommandBuffer& command_buffer, );
			private:
				std::unique_ptr<CVkRenderDevice> m_pDevice{};
				std::unique_ptr<CResourceContainer> m_pResources{};

				utl::vector<FTextureUploadInfo> m_textureUploadInfos{};
				utl::vector<FBufferUploadInfo> m_bufferUploadInfos{};
				utl::vector<FVkTextureLayoutChangeInfo> m_textureLayoutInfos{};

				utl::vector<uint32_t> m_textureIndexRemap{};

				std::queue<FAPIObjectDestroyInfo> m_destroyResource{};

				u64 m_indirectDispatchBuffer{ INVALID_HANDLE };
				u64 m_dispatchBufferOffset{ 0ull };

				// NOTE: global descriptor uses set 0 and binding 0 and 1 (0 is for global uniform, 1 is for texture array)
				FVkGlobalDescriptor m_globalDescriptor{};
				utl::hash_map<u32, utl::unique_queue<u64>> m_textureToUpdate{};

				utl::hash_map<std::thread::id, FVkThreadContext> m_perThreadContext{};
				bool m_bIsFrameStarted{ false };

				FRenderFrameStats m_renderStats{};
				utl::hash_map<EQueueTypeFlagBits, u32> m_zoneEntries{};
				FVkTimestampQuery m_timestampQuery{};

				utl::vector<FVkTexture> m_backbuffer{};

				u64 m_utilizationCounter{ 0ull };

				i32 m_iSurfaceWidth{}, m_iSurfaceHeight{};
			};
		}
	}
}