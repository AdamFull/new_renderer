#pragma once

#include <VkRenderDevice.h>
#include <VkShaderReflection.h>
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
			class Sampler;
			class Texture;
			class Buffer;
			class CommandPool;
			class CommandBuffer;
			class RenderContext;
			class UploadContext;
			class Shader;

			using vk_resource_container_t = CResourceContainer<Texture, Shader, Buffer>;

			class VkRenderBackend : public IRenderBackend
			{
			public:
				~VkRenderBackend() override;

				void Create(const FRenderContextCreateInfo& context_ci) override;
				void CreateGlobalDescriptor(const FGlobalUniformCreateInfo& uniform_ci) override;
				void UpdateGlobalDescriptorData(const FGlobalUniformUpdateInfo& data_view) override;
				const utl::vector<uint32_t>& GetGlobalTextureRemap() override;

				FAPIHandle CreateTexture(const std::string& name, const FTextureCreateInfo& texture_ci) override;
				FAPIHandle CreateTexture(const std::string& name, const FTextureCreateInfo& texture_ci, const FTextureUploadInfo& upload_info) override;
				FAPIHandle GetTextureId(const std::string& name) override;
				FTextureView GetTextureView(FAPIHandle texture_id) override;
				void DestroyTexture(FAPIHandle resource_id) override;
				void ReadPixel(FAPIHandle resource_id, u32 x, u32 y, void* data) override;


				FAPIHandle CreateShader(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci, const std::vector<FShaderCreationBlockInfo>& shader_stages) override;
				FAPIHandle CreateShader(const std::string& name, const FComputeShaderCreateInfo& shader_ci, const FShaderCreationBlockInfo& shader_stage) override;
				FAPIHandle CreateShader(const std::string& name, const FReyTracingShaderCreateInfo& shader_ci) override;
				FShaderReflectionData GetShaderReflection(FAPIHandle shader_view) override;
				void UpdateUniform(FAPIHandle shader_id, uint64_t reflection_name, std::span<const char> data) override;
				void BindTexture(FAPIHandle shader_id, uint64_t reflection_name, FAPIHandle texture_id, uint32_t mip_level) override;
				void BindBuffer(FAPIHandle shader_id, uint64_t reflection_name, FAPIHandle buffer_id) override;
				void FlushConstantRanges(FAPIHandle shader_id, const utl::hash_map<u64, FUniformDataView>& constants, EQueueTypeFlagBits queue_type) override;
				void BindShader(FAPIHandle shader_id, EQueueTypeFlagBits queue_type) override;
				void ResetShaderResources(FAPIHandle shader_id) override;
				void BindShaderResources(FAPIHandle shader_id, EQueueTypeFlagBits queue_type) override;
				void DestroyShader(FAPIHandle resource_id) override;


				FAPIHandle CreateBuffer(const std::string& name, const FBufferCreateInfo& buffer_ci) override;
				void UpdateBuffer(const std::string& name, FAPIHandle buffer_id, const FBufferCreateInfo& buffer_ci) override;
				void BeginUpdateBuffer(BufferUpdateInfo& update_info) override;
				void EndUpdateBuffer(BufferUpdateInfo& update_info) override;
				FBufferView GetBufferView(FAPIHandle buffer_id) override;
				void BindVertexBuffers(const FVertexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type) override;
				void BindIndexBuffer(const FIndexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type) override;
				void MapBufferMemory(FAPIHandle buffer_id, void*& memory) override;
				void UnmapBufferMemory(FAPIHandle buffer_id) override;
				void FlushBuffer(FAPIHandle buffer_id, u64 size, u64 offset) override;
				void DestroyBuffer(FAPIHandle resource_id) override;

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

				void UpdateSurface();

				const std::unique_ptr<RenderDevice>& getDevice() const;

				void BeginFrame() override;
				void EndFrame() override;
				
				bool IsSwapChainFinished() override;

				void BeginDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color = 0xffffffff) override;
				void InsertDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color = 0xffffffff) override;
				void EndDebugLabel(EQueueTypeFlagBits queue_type) override;
			protected:
				void ProcessResourceUtilization();

				void ProcessMemoryBarriers(CommandBuffer& command_buffer, const FVkTranslationInfo& translation_info);
				void ProcessImageLayoutTranslation(CommandBuffer& command_buffer, const vk::ImageMemoryBarrier2KHR& image_translation_info);
				void TextureLayoutTranslation(CommandBuffer& command_buffer, Texture& texture, const FVkTextureTranslationInfo& translation_info);

				//void blitImage(const FVkCommandBuffer& command_buffer, );
			private:
				std::unique_ptr<RenderDevice> render_device{};
				std::unique_ptr<vk_resource_container_t> render_resources;

				utl::vector<std::unique_ptr<Buffer>> upload_buffers;

				utl::vector<uint32_t> texture_index_remap{};

				std::queue<FAPIObjectDestroyInfo> resource_destroy_queue{};

				utl::hash_map<u32, utl::unique_queue<u64>> texture_to_update{};

				std::unique_ptr<CommandPool> command_pool;

				std::unique_ptr<RenderContext> render_context;
				std::unique_ptr<UploadContext> upload_context;

				FRenderFrameStats render_stats{};
				utl::hash_map<EQueueTypeFlagBits, u32> zone_entries{};
				FVkTimestampQuery timestamp_query{};

				utl::hash_map<u32, std::unique_ptr<Sampler>> sampler_cache{};
				std::unique_ptr<Sampler> default_sampler{};

				u64 utilization_counter{ 0ull };
			};
		}
	}
}