#pragma once

#include "VkRenderDevice.h"
#include "VkShaderReflection.h"
#include <engine/interface/ResourceContainer.h>
#include <engine/interface/render/RenderInterface.h>
#include <container/unique_queue.h>
#include <thread>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class Buffer;

			using vk_resource_container_t = CResourceContainer<FVkTexture, FVkShader, Buffer, FVkShaderReflectionData>;

			class VkRenderBackend : public IRenderBackend
			{
			public:
				~VkRenderBackend() override;

				void Create(const FRenderContextCreateInfo& context_ci) override;
				void CreateGlobalDescriptor(const FGlobalUniformCreateInfo& uniform_ci) override;
				void UpdateGlobalDescriptorData(const FGlobalUniformUpdateInfo& data_view) override;
				const utl::vector<uint32_t>& GetGlobalTextureRemap() override;

				FAPIHandle CreateTexture(const std::string& name, const FTextureCreateInfo& texture_ci) override;
				FAPIHandle CreateTexture(const std::string& name, const FTextureCreateInfo& texture_ci, FTextureUploadInfo&& upload_info) override;
				FAPIHandle GetTextureId(const std::string& name) override;
				FTextureView GetTextureView(FAPIHandle texture_id) override;
				void DestroyTexture(FAPIHandle resource_id) override;
				void ReadPixel(FAPIHandle resource_id, u32 x, u32 y, void* data) override;


				FAPIHandle CreateShader(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci, utl::vector<FShaderCreationBlockInfo>&& shader_stages) override;
				FAPIHandle CreateShader(const std::string& name, const FComputeShaderCreateInfo& shader_ci, utl::vector<FShaderCreationBlockInfo>&& shader_stages) override;
				FAPIHandle CreateShader(const std::string& name, const FReyTracingShaderCreateInfo& shader_ci, utl::vector<FShaderCreationBlockInfo>&& shader_stages) override;
				FShaderReflectionData GetShaderReflection(FAPIHandle shader_view) override;
				void UpdateShaderResources(const FShaderResourceCommit& shader_resources, EShaderResourcesUpdateFlags flags = EShaderResourcesUpdateFlagBits::eAll) override;
				void FlushConstantRanges(FAPIHandle shader_id, const utl::hash_map<u64, FUniformDataView>& constants, EQueueTypeFlagBits queue_type) override;
				void BindShader(FAPIHandle shader_id, EQueueTypeFlagBits queue_type) override;
				void BindShaderResources(FAPIHandle shader_id, EQueueTypeFlagBits queue_type) override;
				void DestroyShader(FAPIHandle resource_id) override;


				FAPIHandle CreateBuffer(const std::string& name, const FBufferCreateInfo& buffer_ci) override;
				void UpdateBuffer(const std::string& name, FAPIHandle buffer_id, const FBufferCreateInfo& buffer_ci) override;
				void UploadDataToBuffer(const FBufferUploadInfo& upload_info) override;
				FBufferView GetBufferView(FAPIHandle buffer_id) override;
				void BindVertexBuffers(const FVertexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type) override;
				void BindIndexBuffer(const FIndexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type) override;
				void MapBufferMemory(FAPIHandle buffer_id, void*& memory) override;
				void UnmapBufferMemory(FAPIHandle buffer_id) override;
				void FlushBuffer(FAPIHandle buffer_id, u64 size, u64 offset) override;
				void DestroyBuffer(FAPIHandle resource_id) override;
				void UploadBuffers() override;
			

				void PushG2CBarriers(EQueueTypeFlagBits queue_type, const utl::vector<FAPIHandle>& texture_ids) override;
				void PushC2CBarrier(EQueueTypeFlagBits queue_type) override;
				void PushC2GBarrier(EQueueTypeFlagBits queue_type, const utl::vector<FAPIHandle>& texture_ids) override;

				void ProcessResourceBarriers(EQueueTypeFlagBits queue, const utl::vector<FResourceBarrierInfo>& barriers) override;

				void BeginTimestampZone(EQueueTypeFlagBits queue) override;
				void EndTimestampZone(EQueueTypeFlagBits queue) override;

				void BeginRendering(const FBeginRendering2Info& rendering_info) override;
				void EndRendering(EQueueTypeFlagBits queue) override;

				void BeginCompute(const FBeginComputeInfo& compute_info) override;
				void EndCompute(EQueueTypeFlagBits queue) override;

				void SetViewport(f32 width, f32 height, bool flip_viewport = false) override;
				void SetScissorRect(i32 offset_x, i32 offset_y, u32 width, u32 height) override;
				void SetPrimitiveTopology(EVertexPrimitiveTopology primitivd_topology) override;

				void Draw(EQueueTypeFlagBits queue_type, const FGeometryDrawInfo& draw_info) override;
				void Draw(EQueueTypeFlagBits queue_type, const FIndirectGeometryDrawInfo& draw_info) override;

				void Dispatch(EQueueTypeFlagBits queue_type, const FDispatch2Info& dispatch_group, u64 subgroup_index) override;
				void Dispatch(EQueueTypeFlagBits queue_type, const FDispatchIndirectInfo& dispatch_info) override;

				FDeviceLimitations GetDeviceLimitations() override;
				const FRenderFrameStats& GetRenderFrameStats() const override;
				EAdapterVendor GetDeviceVendor() const override;

				FAPIHandle GetTextureResource(const char* name) override;
				FAPIHandle GetShaderResource(const char* name) override;

				FPoint2D<u32> GetRenderResolution() const override;
				FRect2D<f32> GetRenderRectFromRT(FAPIHandle texture_id) override;
				FRect2D<f32> GetScreenRenderRect() override;

				void UpdateSurface(i32 surface_width, i32 surface_height) override;

				FVkCommandBuffer& CreateCommandBuffer(const FVkCommandBufferCreateInfo& command_buffer_ci, std::thread::id thread_id = std::this_thread::get_id());
				FVkCommandBuffer& GetLastCommandBuffer(EQueueTypeFlagBits queue_type, std::thread::id thread_id = std::this_thread::get_id());
				FVkCommandBuffer& GetCommandBuffer(EQueueTypeFlagBits queue_type, u64 index, std::thread::id thread_id = std::this_thread::get_id());
				utl::vector<FVkCommandBuffer>& GetCommandBuffers(EQueueTypeFlagBits queue_type, std::thread::id thread_id = std::this_thread::get_id());
				void DestroyCommandBuffer(EQueueTypeFlagBits queue_type, std::thread::id thread_id = std::this_thread::get_id());

				const std::unique_ptr<VkRenderDevice>& getDevice() const;

				void BeginFrame() override;
				void EndFrame() override;
				
				bool IsSwapChainFinished() override;

				void BeginDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color = 0xffffffff) override;
				void InsertDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color = 0xffffffff) override;
				void EndDebugLabel(EQueueTypeFlagBits queue_type) override;
			protected:
				void DestroyThreadContext(FVkThreadContext& context);
				void ProcessResourceBarriers();
				void ProcessResourceUpload();
				void ProcessResourceUtilization();
				void UpdateBackbuffer();

				FRenderApiErrorInfo CreateTexture(const FVkTextureCreateInfo& texture_ci, FVkTexture& texture);
				void DestroyTexture(FVkTexture& texture);

				FRenderApiErrorInfo CreateShaderResources(const FVkShaderResourcesInfo& resources_info, FVkShader& shader);
				void DestroyShader(FVkShader& shader);

				void BindShaderDescriptors(const FVkCommandBuffer& command_buffer, const FVkShader& shader);

				void ProcessMemoryBarriers(const FVkCommandBuffer& command_buffer, const FVkTranslationInfo& translation_info);
				void ProcessImageLayoutTranslation(const FVkCommandBuffer& command_buffer, const FVkImageTranslationInfo& image_translation_info);
				void TextureLayoutTranslation(const FVkCommandBuffer& command_buffer, FVkTexture& texture, const FVkTextureTranslationInfo& translation_info);

				void CopyBufferToTexture(const FVkCommandBuffer& command_buffer, const Buffer& buffer, const FVkTexture& texture, const utl::vector<vk::BufferImageCopy2KHR>& copy_regions);
				//void blitImage(const FVkCommandBuffer& command_buffer, );
			private:
				std::unique_ptr<VkRenderDevice> render_device{};
				std::unique_ptr<vk_resource_container_t> render_resources;

				utl::vector<FTextureUploadInfo> texture_upload_infos{};
				utl::vector<FBufferUploadInfo> buffer_upload_infos{};
				utl::vector<FVkTextureLayoutChangeInfo> texture_layout_change_infos{};

				utl::vector<uint32_t> texture_index_remap{};

				std::queue<FAPIObjectDestroyInfo> resource_destroy_queue{};

				// NOTE: global descriptor uses set 0 and binding 0 and 1 (0 is for global uniform, 1 is for texture array)
				FVkGlobalDescriptor global_descriptor{};
				utl::hash_map<u32, utl::unique_queue<u64>> texture_to_update{};

				utl::hash_map<std::thread::id, FVkThreadContext> per_thread_context{};
				bool is_frame_started{ false };

				FRenderFrameStats render_stats{};
				utl::hash_map<EQueueTypeFlagBits, u32> zone_entries{};
				FVkTimestampQuery timestamp_query{};

				utl::vector<FVkTexture> backbuffer{};

				utl::hash_map<u64, vk::Sampler> sampler_cache{};
				vk::Sampler default_sampler{};

				u64 utilization_counter{ 0ull };

				i32 surface_width{}, m_iSurfaceHeight{};
			};
		}
	}
}