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

				void Create(const FRenderContextCreateInfo& context_ci) override;
				void CreateGlobalDescriptor(const FGlobalUniformCreateInfo& uniform_ci) override;
				void UpdateGlobalDescriptorData(const FGlobalUniformUpdateInfo& data_view) override;
				const utl::vector<uint32_t>& GetGlobalTextureRemap() override;

				u64 CreateTexture(const std::string& name, const FTextureCreateInfo& texture_ci, FTextureUploadInfo&& upload_info) override;
				u64 GetTextureId(const std::string& name) override;
				u64 CreateRenderTarget(const std::string& name, const FTextureCreateInfo& texture_ci) override;
				const FTextureCreateInfo& GetTextureInfo(u64 texture_id) override;
				void DestroyTexture(u64 resource_id) override;


				u64 CreateShader(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci, const  utl::vector<FShaderCreationBlockInfo>& shader_stages) override;
				u64 CreateShader(const std::string& name, const FComputeShaderCreateInfo& shader_ci, const  utl::vector<FShaderCreationBlockInfo>& shader_stages) override;
				u64 CreateShader(const std::string& name, const FReyTracingShaderCreateInfo& shader_ci, const  utl::vector<FShaderCreationBlockInfo>& shader_stages) override;
				FShaderReflectionData GetShaderReflection(const u64& shader_view) override;
				void UpdateShaderResources(const FShaderResourceCommit& shader_resources, EShaderResourcesUpdateFlags flags = EShaderResourcesUpdateFlagBits::eAll) override;
				void FlushConstantRanges(u64 shader_id, const utl::hash_map<u64, FUniformDataView>& constants, EQueueTypeFlagBits queue_type) override;
				void BindShader(u64 shader_id, EQueueTypeFlagBits queue_type) override;
				void DestroyShader(u64 resource_id) override;


				u64 CreateBuffer(const std::string& name, const FBufferCreateInfo& buffer_ci) override;
				void UpdateBuffer(const std::string& name, u64 buffer_id, const FBufferCreateInfo& buffer_ci) override;
				void UploadDataToBuffer(const FBufferUploadInfo& upload_info) override;
				FBufferView GetBufferView(u64 buffer_id) override;
				void BindVertexBuffers(const FVertexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type) override;
				void BindIndexBuffer(const FIndexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type) override;
				void MapBufferMemory(u64 buffer_id, void*& memory) override;
				void UnmapBufferMemory(u64 buffer_id) override;
				void FlushBuffer(u64 buffer_id, u64 size, u64 offset) override;
				void DestroyBuffer(u64 resource_id) override;
				void UploadBuffers() override;
			

				void PushG2CBarriers(EQueueTypeFlagBits queue_type, const utl::vector<u64>& texture_ids) override;
				void PushC2CBarrier(EQueueTypeFlagBits queue_type) override;
				void PushC2GBarrier(EQueueTypeFlagBits queue_type, const utl::vector<u64>& texture_ids) override;

				void ProcessResourceBarriers(EQueueTypeFlagBits queue, const utl::vector<FResourceBarrierInfo>& barriers) override;

				void BeginTimestampZone(EQueueTypeFlagBits queue) override;
				void EndTimestampZone(EQueueTypeFlagBits queue) override;

				void BeginRendering(const FBeginRendering2Info& rendering_info) override;
				void EndRendering(EQueueTypeFlagBits queue) override;

				void BeginCompute(const FBeginComputeInfo& compute_info) override;
				void EndCompute(EQueueTypeFlagBits queue) override;

				void SetViewport(f32 width, f32 height, bool flip_viewport = false) override;
				void SetScissorRect(i32 offset_x, i32 offset_y, u32 width, u32 height) override;

				void Draw(EQueueTypeFlagBits queue_type, const FGeometryDrawInfo& draw_info) override;
				void Draw(EQueueTypeFlagBits queue_type, const FIndirectGeometryDrawInfo& draw_info) override;

				void Dispatch(EQueueTypeFlagBits queue_type, const FDispatch2Info& dispatch_group, u64 subgroup_index) override;
				void Dispatch(EQueueTypeFlagBits queue_type, const FDispatchIndirectInfo& dispatch_info) override;

				FDeviceLimitations GetDeviceLimitations() override;
				const FRenderFrameStats& GetRenderFrameStats() const override;
				EAdapterVendor GetDeviceVendor() const override;

				u64 GetTextureResource(const char* name) override;
				u64 GetShaderResource(const char* name) override;

				FPoint2D<u32> GetRenderResolution() const override;
				FRect2D<f32> GetRenderRectFromRT(u64 texture_id) override;
				FRect2D<f32> GetScreenRenderRect() override;

				u32 GetDepthFormat(bool depth_only = false) override;
				u32 GetApiFormat(EApiDataFormat global_format) override;
				u32 GetPresentFormat() override;

				void UpdateSurface(i32 surface_width, i32 surface_height) override;

				void GetNativeContext(void* context) override;

				FVkCommandBuffer& CreateCommandBuffer(const FVkCommandBufferCreateInfo& command_buffer_ci, std::thread::id thread_id = std::this_thread::get_id());
				FVkCommandBuffer& getLastCommandBuffer(EQueueTypeFlagBits queue_type, std::thread::id thread_id = std::this_thread::get_id());
				FVkCommandBuffer& GetCommandBuffer(EQueueTypeFlagBits queue_type, u64 index, std::thread::id thread_id = std::this_thread::get_id());
				utl::vector<FVkCommandBuffer>& getCommandBuffers(EQueueTypeFlagBits queue_type, std::thread::id thread_id = std::this_thread::get_id());
				void DestroyCommandBuffer(EQueueTypeFlagBits queue_type, std::thread::id thread_id = std::this_thread::get_id());

				const std::unique_ptr<CVkRenderDevice>& getDevice() const;

				void BeginFrame() override;
				void EndFrame() override;
				
				bool IsSwapChainFinished() override;

				void BeginDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color = 0xffffffff) override;
				void InsertDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color = 0xffffffff) override;
				void EndDebugLabel(EQueueTypeFlagBits queue_type) override;
			protected:
				void destroyThreadContext(FVkThreadContext& context);
				void ProcessResourceBarriers();
				void processResourceUpload();
				void processResourceUtilization();
				void updateBackbuffer();

				FVkBufferCreateInfo makeBufferCreateInfo(const std::string& name, const FBufferCreateInfo& buffer_ci);

				FRenderApiErrorInfo CreateTexture(const FVkTextureCreateInfo& texture_ci, FVkTexture& texture);
				void DestroyTexture(FVkTexture& texture);

				FRenderApiErrorInfo createShaderResources(const FVkShaderResourcesInfo& resources_info, FVkShader& shader);
				void DestroyShader(FVkShader& shader);

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

				utl::hash_map<u64, vk::Sampler> m_samplerCache{};
				vk::Sampler m_defaultSampler{};

				u64 m_utilizationCounter{ 0ull };

				i32 m_iSurfaceWidth{}, m_iSurfaceHeight{};
			};
		}
	}
}